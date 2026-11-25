#pragma once
// Kunci jawaban. Hanya Red (tulis) dan Purple (baca: evaluation, menu) yang
// boleh meng-include file ini. detector.cpp DILARANG (model.md 4.5).
//
// Penyimpanannya didefinisikan di dataset.cpp; label[k] berpasangan dengan dataset[k].
#include <vector>

#include "types.h"

extern std::vector<LabelSet> label;
