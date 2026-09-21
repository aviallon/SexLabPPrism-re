// Catalog native bodies and catalogue state live in src/Papyrus/Natives.cpp's
// GLOBAL anonymous namespace so their baked __FUNCSIG__/source_loc match the
// original (`anonymous-namespace'::Papyrus_CatalogBegin ...`). This TU is kept
// only so the file list stays stable; the cross-TU accessor `Catalog::RetryPublish`
// and `Catalog::Fnv1aHash::operator()` are defined there too.
