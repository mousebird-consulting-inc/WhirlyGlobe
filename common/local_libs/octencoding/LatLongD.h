#ifndef LatLongD_h
#define LatLongD_h

/** Use: Instantiate a LatLongD object and then call encode/decode on it. */

#include "common.h"
#include "snorm.h"
#include <stdlib.h> // For malloc

class LatLongD {
protected:
    
    double EPSILON;
    int  N_phi;
    unsigned int* N_th; // size is N_phi
    unsigned int* sumN_th;
    int bitCount;   
    
    double determineEpsilonFromBitLimit(const int bits) {
        /**Finds minimal epsilon which can be represented in bits - note that bits required to store a minmal error does not increase monotonically*/
        double MAX = .09;
        double MIN = 0;
        double epsilon = (MAX + MIN) / 2;
        while (true) {
            initializeFromEpsilon(epsilon);
            if (bitCount == bits) {
                MAX = epsilon;
                epsilon = (MAX + MIN) / 2;
            } else if (bitCount > bits || bitCount < 1) {
                MIN = epsilon;
                epsilon = (MAX + MIN) / 2;
            } else if (bitCount < bits) { 
                MAX = epsilon;
                epsilon = (MAX + MIN) / 2;
            }
            if (fabs(MAX - MIN) <= .0000001) {;
                return MAX;
            }
        }
        return 0;
    }

    inline double phi_hat(const unsigned int j) const {
        return (j * PI) / (N_phi - 1);
    }

    int N_theta(const unsigned int j) const {
        // Used to compute the initial value, which will then be stored into the N_th array
      if (j == 0 || j == (unsigned int)(N_phi - 1)) {
            //at the poles, phi hat will be 0 or pi, and the below expression will compute NaN for N_theta(j) as sin(ph) = 0
            //so, we return 1, the only number of points that can exist at a pole
            return 1;
        }
        const double ph = phi_hat(j);
        const double worstAngle = ph < PI / 2 ? ph + (PI / (2 * (N_phi - 1))) : ph - (PI / (2 * (N_phi - 1)));
        const double num = cos(EPSILON) - cos(ph)*cos(worstAngle);
        const double denom = sin(ph)*sin(worstAngle);
        if (num >= denom) {
            return -1;
        }
        double arc = ::acos( num / denom); //need num / denom to not be > 1
        return (int)ceil(PI / arc);
    }

    unsigned long long totalPoints(int Nphi) {
        unsigned long long sum = 0;
        N_phi = Nphi;
        for (int j = 0; j < Nphi; ++j) {
            int n = N_theta(j);
            if (n > 0) { 
                sum += n;
            } else { //overflow, N_theta[j] too large to be held in a 32-bit int
                return 0;
            }
        }
        return sum;
    }

    int llog2(unsigned long long x) {
        if (x < (1LL << 31)) {
	    return (int)ceil(log(floor((float)x)));
        } else if (x < (1LL << 32)) {
            return 32;
        } else { 
            return 0;
        }
    }

    void initializeFromEpsilon(const double epsilon) {
        EPSILON = epsilon;
        N_phi = int(PI / (2.0 * EPSILON)) + 3; //baseline, below this, maximum error can be greater than epsilon
        int MIN = N_phi;
        int MAX = 2 * N_phi;
        int MID = 0;
        bool done = false;
        while(!done) {
            MID = (MAX+MIN)/2;
            unsigned long long y_1 = totalPoints(MIN);
            unsigned long long y_2 = totalPoints(MAX);
            unsigned long long y_mid = totalPoints(MID);

            int m_1 = (int)(y_mid - y_1)   / (MID - MIN);
            int m_2 = (int)(y_2   - y_mid) / (MAX - MID);

            if (MAX - MIN <= 2) {
                N_phi = MID;
                done = true;
                bitCount = llog2(y_mid);
            }
            else if (m_1 <= 0 && m_2 <= 0) {
                MIN = MID;
            } else if (m_1 >= 0 && m_2 >= 0) {
                MAX = MID;
            } else {
                MIN = (MID + MIN) / 2;
                MAX = (MAX + MID) / 2;
            }
        }

        N_th = (unsigned int*)malloc(N_phi * sizeof(unsigned int));
        sumN_th = (unsigned int*)malloc(N_phi * sizeof(unsigned int));
        sumN_th[0] = 1;
        N_th[0] = 1;
        for (unsigned int j = 1; j < (unsigned int)N_phi; ++j) {
            N_th[j] = N_theta(j);    
            sumN_th[j] = N_th[j] + sumN_th[j - 1];
        }
    }

    unsigned int indexFromJK(const unsigned int j, const unsigned int k) const{
        return k + (j == 0 ? 0 : sumN_th[j - 1]);
    }

    void decodeFromJK(unsigned int j, unsigned int k, float vec[3]) const {
        const float phat = (float)phi_hat(j);
        const float thhat =  (k * 2.0f * PI) / N_th[j];
    
        // basic spherical coordinate transformations
        const float temp = sin(phat) * cos(thhat);
        vec[0] = temp;
        vec[1] = temp * tan(thhat);
        vec[2] = cos(phat);
    }

public:
    LatLongD(int bits) {
        bitCount = bits;
        initializeFromEpsilon(determineEpsilonFromBitLimit(bits));
    }


    void encode(const float vec[3], unsigned int* compressed) {
        //basic spherical coordinate transformations
        float theta = atan2(vec[1], vec[0]);
        theta += theta < 0 ? (2.0f * PI) : 0.0f ;//map from [-pi, pi] to [0, 2pi]
        const float phi   = ::acos(vec[2]);
        //j and k are indices which allow recovery of coordinates using N_phi and N_theta
        unsigned int  j  = (int)( phi * (N_phi - 1.0f) / PI + .5f);

        const int N_t = N_th[j];
        unsigned int k = (int)(theta*N_t / (2.0f * PI) + .5f ) % N_t;

        /** To further compress, we map (j,k) to a 1-dimensional index into the list of all points */
        *compressed = indexFromJK(j, k);
    }

    void decode(const unsigned int p, float vec[3]) const {
    
        unsigned int min = 1;
        unsigned int max = N_phi - 1;
        bool done = false;
        unsigned int j = (N_phi - 1)/2;
        if (p == 0) {
            j = 0;
        } else if (p == sumN_th[N_phi - 1]) {
            j = N_phi - 1;
        } else {
            while(!done) {
                int diff = p - sumN_th[j - 1];
                if (diff >= 0) {
		  if (j == (unsigned int)(N_phi - 1) || (unsigned int)diff < N_th[j]) {
                        done = true;
                    } else {
                        min = j + 1;
                        j = int(ceil(float((max + j)) / 2.0f));
                    }
                } else {
                    max = j;
                    j = (j + min) / 2;
                }
            }
        }
        const unsigned int k = p - (unsigned int)(j == 0 ? 0 : sumN_th[j - 1]);
        decodeFromJK(j, k, vec);
    }
};

#endif
