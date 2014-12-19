//
//  MaplyBaseViewController_ext.swift
//

import UIKit

extension MaplyBaseViewController {

    func addLabels(labels: [MaplyLabel], attributes attr: [MaplySharedAttribute], threadMode: MaplyThreadMode = MaplyThreadAny) -> MaplyComponentObject {
        return addLabels(labels, desc: MaplySharedAttribute.dictionaryFromArray(attr), mode: threadMode)
    }
    
    func addScreenLabels(labels: [MaplyScreenLabel], attributes attr: [MaplySharedAttribute], threadMode: MaplyThreadMode = MaplyThreadAny) -> MaplyComponentObject {
        return addScreenLabels(labels, desc: MaplySharedAttribute.dictionaryFromArray(attr), mode: threadMode)
    }
    
    func addScreenMarkers(markers: [MaplyScreenMarker], attributes attr: [MaplySharedAttribute], threadMode: MaplyThreadMode = MaplyThreadAny) -> MaplyComponentObject {
        return addScreenMarkers(markers, desc: MaplySharedAttribute.dictionaryFromArray(attr), mode: threadMode)
    }
    
    func addMarkers(markers: [MaplyMarker], attributes attr: [MaplySharedAttribute], threadMode: MaplyThreadMode = MaplyThreadAny) -> MaplyComponentObject {
        return addMarkers(markers, desc: MaplySharedAttribute.dictionaryFromArray(attr), mode: threadMode)
    }
    
    func addVectors(vectors: [MaplyVectorObject], attributes attr: [MaplySharedAttribute], mode threadMode: MaplyThreadMode = MaplyThreadAny) -> MaplyComponentObject {
        return addVectors(vectors, desc: MaplySharedAttribute.dictionaryFromArray(attr), mode: threadMode)
    }
    
}
