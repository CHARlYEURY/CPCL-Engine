#pragma once

// Borne v dans [lo, hi]  (ancienne ZBuffer::clampi, ce n'est pas une méthode de classe)
inline int clampi(int v, int lo, int hi)
{
	return v < lo ? lo : (v > hi ? hi : v);
}
