#pragma once
struct bytes : public std::shared_ptr<void> {
public:
	unsigned int lenght;
	bytes(std::nullptr_t) :
		std::shared_ptr<void>(nullptr),
		lenght(0) { }
	bytes(int len) :
		std::shared_ptr<void>(malloc(len), free),
		lenght(len) { }
};

struct AssetDescriptor {
	int32_t descriptor = -1;
	off_t start = 0;
	off_t end = 0;
};

class Asset {
public:
	static bytes read(const std::string& name) {
		AAsset* asset = AAssetManager_open(engine.app->activity->assetManager, name.c_str(), AASSET_MODE_UNKNOWN);
		if (NULL == asset) {
			LOGE("_ASSET_NOT_FOUND_: %s", name.c_str());
			return nullptr;
		}

		long size = AAsset_getLength(asset);
		bytes buffer(size);

		AAsset_read(asset, buffer.get(), size);
		AAsset_close(asset);

		return buffer;
	}

	AAsset* asset = nullptr;

	bool is_open() {
		return asset != nullptr;
	}

	Asset(const std::string& name) {
		asset = AAssetManager_open(engine.app->activity->assetManager, name.c_str(), AASSET_MODE_UNKNOWN);
		if (NULL == asset) {
			LOGE("_ASSET_NOT_FOUND_: %s", name.c_str());
		}
	}

	~Asset() {
		close();
	}

	void close() {
		if (asset) {
			AAsset_close(asset);
			asset = nullptr;
		}
	}

	AssetDescriptor descript() {
		AssetDescriptor asset_descriptor;
		asset_descriptor.descriptor = AAsset_openFileDescriptor(asset, &asset_descriptor.start, &asset_descriptor.end);
		return asset_descriptor;
	}

	int read(void* buf, int max_len) {
		return AAsset_read(asset, buf, max_len);
	}
	int size() {
		return AAsset_getLength(asset);
	}
};