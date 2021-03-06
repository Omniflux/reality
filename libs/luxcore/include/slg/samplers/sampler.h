/***************************************************************************
 * Copyright 1998-2015 by authors (see AUTHORS.txt)                        *
 *                                                                         *
 *   This file is part of LuxRender.                                       *
 *                                                                         *
 * Licensed under the Apache License, Version 2.0 (the "License");         *
 * you may not use this file except in compliance with the License.        *
 * You may obtain a copy of the License at                                 *
 *                                                                         *
 *     http://www.apache.org/licenses/LICENSE-2.0                          *
 *                                                                         *
 * Unless required by applicable law or agreed to in writing, software     *
 * distributed under the License is distributed on an "AS IS" BASIS,       *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.*
 * See the License for the specific language governing permissions and     *
 * limitations under the License.                                          *
 ***************************************************************************/

#ifndef _SLG_SAMPLER_H
#define	_SLG_SAMPLER_H

#include <string>
#include <vector>

#include "luxrays/core/randomgen.h"
#include "slg/slg.h"
#include "slg/film/film.h"

namespace slg {

//------------------------------------------------------------------------------
// OpenCL data types
//------------------------------------------------------------------------------

namespace ocl {
#include "slg/samplers/sampler_types.cl"
}

//------------------------------------------------------------------------------
// Sampler
//------------------------------------------------------------------------------

typedef enum {
	RANDOM = 0,
	METROPOLIS = 1,
	SOBOL = 2
} SamplerType;

class Sampler {
public:
	Sampler(luxrays::RandomGenerator *rnd, Film *flm) : rndGen(rnd), film(flm) { }
	virtual ~Sampler() { }

	virtual SamplerType GetType() const = 0;
	virtual void RequestSamples(const u_int size) = 0;

	// index 0 and 1 are always image X and image Y
	virtual float GetSample(const u_int index) = 0;
	virtual void NextSample(const std::vector<SampleResult> &sampleResults) = 0;

	static SamplerType String2SamplerType(const std::string &type);
	static const std::string SamplerType2String(const SamplerType type);

protected:
	void AddSamplesToFilm(const std::vector<SampleResult> &sampleResults, const float weight = 1.f) const {
		for (std::vector<SampleResult>::const_iterator sr = sampleResults.begin(); sr < sampleResults.end(); ++sr)
			film->SplatSample(*sr, weight);
	}

	luxrays::RandomGenerator *rndGen;
	Film *film;
};

}

#endif	/* _SLG_SAMPLER_H */
