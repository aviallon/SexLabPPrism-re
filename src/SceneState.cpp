// SceneState native bodies and session state live in src/Papyrus/Natives.cpp's
// GLOBAL anonymous namespace so their baked __FUNCSIG__/source_loc match the
// original (`anonymous-namespace'::Papyrus_PublishSceneState ...`). This TU is
// kept only so the file list stays stable; the cross-TU presentation accessors
// (IsSceneActive/IsUiMode/... ) are defined there too.
