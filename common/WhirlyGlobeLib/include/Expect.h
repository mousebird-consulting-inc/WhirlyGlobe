/*  Expect.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 7/22/2021
 *  Copyright 2021-2021 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

// EXPECT provides the compiler with branch prediction information.
// EXPECT takes two integral arguments: a value, and the expected result, and evaluates to the former.
// The compiler uses the `builtin-expect-probability` parameter as the probability, defaulting to 0.9.
// EXPECT_WITH_PROBABILITY allows specifying the probability directly.

#if !defined(EXPECT)
# if (defined(__clang__) && __has_builtin(expect)) || (__GNUC__ > 3)
#  define EXPECT(x,v) __builtin_expect(!!(x), v)
# else
#  define EXPECT(x,v) (x)
# endif
#endif

#if !defined(EXPECT_WITH_PROBABILITY)
# if defined(__clang__) && defined(__has_builtin)
#  define EXPECT_WITH_PROBABILITY(x,b,p) __builtin_expect_with_probability(!!(x), v, p)
# else
#  define EXPECT_WITH_PROBABILITY(x,v,p) (x)
# endif
#endif

#if !defined(LIKELY)
# define LIKELY(x) EXPECT(x, 1)
#endif
#if !defined(UNLIKELY)
# define UNLIKELY(x) EXPECT(x, 0)
#endif

#if !defined(LIKELY)
# define LIKELY(x) EXPECT(x, 1)
#endif
#if !defined(UNLIKELY)
# define UNLIKELY(x) EXPECT(x, 0)
#endif

