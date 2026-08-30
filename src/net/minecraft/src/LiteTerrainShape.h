#pragma once

namespace LiteTerrain
{
struct BiomeShape
{
	float baseHeight;
	float heightVariation;
};

struct BiomeBlendSample
{
	BiomeShape shape;
	float inverseWeightDenominator;
};

inline float clamp(float value, float minimum, float maximum)
{
	if (value < minimum)
		return minimum;
	if (value > maximum)
		return maximum;
	return value;
}

inline BiomeBlendSample makeBiomeBlendSample(const BiomeShape &shape)
{
	const float denominator = clamp(shape.baseHeight + 2.0f, 0.25f, 4.0f);
	return BiomeBlendSample{shape, 1.0f / denominator};
}

template <typename Getter>
BiomeShape blendBiomeShape(int centerX, int centerZ, Getter getBiomeSample)
{
	static constexpr float kernel[25] = {
		1.0f, 2.0f, 3.0f, 2.0f, 1.0f,
		2.0f, 4.0f, 6.0f, 4.0f, 2.0f,
		3.0f, 6.0f, 9.0f, 6.0f, 3.0f,
		2.0f, 4.0f, 6.0f, 4.0f, 2.0f,
		1.0f, 2.0f, 3.0f, 2.0f, 1.0f
	};

	const BiomeShape center = getBiomeSample(centerX, centerZ).shape;
	float baseHeight = 0.0f;
	float heightVariation = 0.0f;
	float totalWeight = 0.0f;

	for (int offsetZ = -2; offsetZ <= 2; ++offsetZ)
	{
		for (int offsetX = -2; offsetX <= 2; ++offsetX)
		{
			const BiomeBlendSample &neighbor = getBiomeSample(centerX + offsetX, centerZ + offsetZ);
			const int kernelIndex = (offsetZ + 2) * 5 + offsetX + 2;
			float weight = kernel[kernelIndex] * neighbor.inverseWeightDenominator;
			if (neighbor.shape.baseHeight > center.baseHeight)
				weight *= 0.5f;

			baseHeight += neighbor.shape.baseHeight * weight;
			heightVariation += neighbor.shape.heightVariation * weight;
			totalWeight += weight;
		}
	}

	if (totalWeight <= 0.0f)
		return center;
	return BiomeShape{baseHeight / totalWeight, heightVariation / totalWeight};
}

inline float surfaceHeight(float baseHeight, float amplitude, float continentalNoise,
	float detailNoise, const BiomeShape &biome)
{
	const float biomeOffset = clamp(biome.baseHeight * 16.0f, -18.0f, 24.0f);
	float relief = clamp(biome.heightVariation * 0.75f + 0.15f, 0.15f, 1.15f);
	if (biome.baseHeight < 0.0f)
		relief *= 0.55f;

	const float detailAmplitude = amplitude * (0.12f + relief * 0.12f);
	return baseHeight + biomeOffset + continentalNoise * amplitude * relief +
		detailNoise * detailAmplitude;
}
}
