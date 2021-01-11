#pragma once
#include "stdafx.h"

struct VectorLong
{
	long X, Y, Z;

	VectorLong(void)
	{
	}

	VectorLong(const long x, const long y, const long z)
	{
		X = x;
		Y = y;
		Z = z;
	}

	VectorLong operator +(const VectorLong& A) const
	{
		return VectorLong(X + A.X, Y + A.Y, Z + A.Z);
	}

	VectorLong operator +(const long A) const
	{
		return VectorLong(X + A, Y + A, Z + A);
	}

	VectorLong operator *(const long A) const
	{
		return VectorLong(A * X, A * Y, A * Z);
	}

	VectorLong operator *(const VectorLong& A) const
	{
		return VectorLong(A.X * X, A.Y * Y, A.Z * Z);
	}

	VectorLong operator -(const long A) const
	{
		return VectorLong(A * X, A * Y, A * Z);
	}

	VectorLong operator -(const VectorLong& A) const
	{
		return VectorLong(A.X - X, A.Y - Y, A.Z - Z);
	}

	VectorLong operator /(const long A) const
	{
		return VectorLong(A / X, A / Y, A / Z);
	}

	VectorLong operator /(const VectorLong& A) const
	{
		return VectorLong(A.X / X, A.Y / Y, A.Z / Z);
	}

	long dot(const VectorLong& vec) const
	{
		return X * vec.X + Y * vec.Y + Z * vec.Z;
	}

	long lengthSquared()
	{
		return X * X + Y * Y + Z * Z;
	}

	long length()
	{
		return static_cast<long>(sqrt(lengthSquared()));
	}

	VectorLong perpendicularTo()
	{
		return VectorLong(Z, Y, -X);
	}

	VectorLong Normalize()
	{
		long length = this->length();
		if (length != 0)
		{
			long inv = 1.0L / length;
			X *= inv;
			Y *= inv;
			Z *= inv;
		}
		return VectorLong(X, Y, Z);
	}

	long DistTo(const VectorLong& A)
	{
		long out = sqrtf(powf(X - A.X, 2) + powf(Y - A.Y, 2) + powf(Z - A.Z, 2));
		return out < 0 ? out * -1 : out;
	}
};
