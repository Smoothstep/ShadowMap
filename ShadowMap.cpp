constexpr int32_t TerrainW = 2000;
constexpr int32_t TerrainH = 2000;

constexpr int32_t DownsampleFactor = 4;

File::CFile HeightMapFile(L"heightmap.raw");
File::CFile ShadowMapFile(L"Shadowmap.raw");

HeightMapFile.ReadFileContent();

ShadowMapFile.GetContentRef().ResizeUninitialized
(
	TerrainW / DownsampleFactor *
	TerrainH / DownsampleFactor
);

Uint8  * ShadowMap = new Uint8[HeightMapFile.GetContentRef().size() / sizeof(Uint16)];
Uint16 * HeightMap = reinterpret_cast<Uint16*>(HeightMapFile.GetContentRef().data());

std::fill(ShadowMap, ShadowMap + TerrainW * TerrainH, 255);

Vector3f S(0.0f, 0.0f, 1000.0f);
Vector3f P;
Vector3f D;

Vector2f Size(TerrainW, TerrainH);
Vector2f Steps;
Vector3f SunTarget(1000.0f, 1000.0f, 0);
Vector3f L = (SunTarget - S).GetSafeNormal();

float End;
constexpr Float HeightScale = 0.01;

for (int Y = 0; Y < TerrainH; ++Y)
{
	for (int X = 0; X < TerrainW; ++X)
	{
		P.X = X;
		P.Y = Y;

		if (S.X == P.X && S.Y == P.Y)
		{
			continue;
		}

		P.Z = HeightMap[X + Y * TerrainW] * HeightScale;

		D = (P - S);
		D.Normalize();

		float DotDL = Math::Clamp(L | D, SMALL_NUMBER, 1.0f);

		Float R = Math::Abs(D.X) > Math::Abs(D.Y) ? 1.0 / D.X : 1.0 / D.Y;

		Vector3f Step = DotDL * Vector3f(D.X * R, D.Y * R, D.Z);

		float Len = P.Z / (Math::Max(1.0f, -Step.Z));

		// Calculate bounds.

		End = P.X + Step.X * Len;

		if (End >= Size.X)
		{
			Len = (Size.X - P.X) * Step.X;
		}
		else if (End < 0)
		{
			Len = P.X * -Step.X;
		}

		End = P.Y + Step.Y * Len;

		if (End >= Size.Y)
		{
			Len = (Size.Y - P.Y) * Step.Y;
		}
		else if (End < 0)
		{
			Len = P.Y * -Step.Y;
		}

		Len -= 1.0f;
		float LenEstimated = Len;
		float Z = P.Z;

		while (Len > 0)
		{
			P += Step;

			int IX = P.X;
			int IY = P.Y;

			int O = IX + IY * TerrainW;

			float Sample = HeightMap[O] * HeightScale + 0.1f;

			if (P.Z >= Sample)
			{
				int L = O + 1;
				int R = O - 1;
				int T = O - TerrainW;
				int B = O + TerrainW;
				int LT = O - TerrainW + 1;
				int RT = O - TerrainW - 1;
				int LB = O + TerrainW + 1;
				int LR = O + TerrainW - 1;

				Uint8 Shadow = 256 / 256 * (LenEstimated - Len);
				Uint8 ShadowAvg = Shadow;

				if (L > -1)
				{
					ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[L] / 8);
					ShadowMap[L] = Math::Max(256, ShadowMap[L] + Shadow / 8);
				}
				if (T > -1)
				{
					ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[T] / 8);
					ShadowMap[T] = Math::Max(256, ShadowMap[T] + Shadow / 8);
				}
				if (LT > -1)
				{
					ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[LT] / 8);
					ShadowMap[LT] = Math::Max(256, ShadowMap[LT] + Shadow / 8);
				}

				ShadowMap[O] = ShadowAvg;
			}

			Len -= 1.0f;
		}
	}
}

DownsampleShadowMap(
	ShadowMap,
	ShadowMapFile.GetContentRef().data(),
	TerrainW,
	TerrainH,
	TerrainW / DownsampleFactor,
	TerrainH / DownsampleFactor);

ShadowMapFile.WriteFileContent();

delete[] ShadowMap;