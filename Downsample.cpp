void DownsampleShadowMap(Uint8 * Data, Uint8 * Downsampled, int32_t Width, int32_t Height, int32_t NewWidth, int32_t NewHeight)
{
	const float RatioX = Width / NewWidth;
	const float RatioY = Height / NewHeight;

	const int32_t SX = Math::Ceil(RatioX / 2.0f);
	const int32_t SY = Math::Ceil(RatioY / 2.0f);

	const int32_t MX = Math::Floor(RatioX);
	const int32_t MY = Math::Floor(RatioY);

	const float RX = (RatioX - MX) / 2.0f;
	const float RY = (RatioY - MY) / 2.0f;

	const float MaxDistance = Vector2f::Distance(Vector2f(0, 0), Vector2f(SX, SY));

	for (int32_t Y = 0; Y < NewHeight; ++Y)
	{
		for (int32_t X = 0; X < NewWidth; ++X)
		{
			int32_t TranslatedX = RatioX * X;
			int32_t TranslatedY = RatioY * Y;

			Uint8 Sample;
			Uint8 Lerp = Data[TranslatedX + TranslatedY * Width];

			int32_t MLX = TranslatedX - SX;
			int32_t MRX = TranslatedX + SX;
			int32_t MLY = TranslatedY - SY;
			int32_t MRY = TranslatedY + SY;

			for (int32_t
				LY = Math::Max(MLY, 0);
				LY < Math::Min(MRY, Height); ++LY)
			{
				for (int32_t
					LX = Math::Max(MLX, 0);
					LX < Math::Min(MRX, Width); ++LX)
				{
					Sample = Data[LX + LY * Height];

					float LerpFactor = 0.5f - 0.5f * (Vector2f::Distance(Vector2f(TranslatedX, TranslatedY), Vector2f(LX, LY)) / MaxDistance);

					if (LX == MLX || LX == MRX)
					{
						LerpFactor -= RX;
					}

					if (LY == MLY || LY == MRY)
					{
						LerpFactor -= RY;
					}

					Lerp = Lerp + (Sample - Lerp) * LerpFactor;
				}
			}

			Downsampled[X + Y * NewWidth] = Lerp;
		}
	}
}