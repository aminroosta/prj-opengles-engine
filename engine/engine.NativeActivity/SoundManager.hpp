#pragma once

class SoundManager; /* forward declaration */

class Sound {
public:
	Sound(const std::string& path_) : path(path_) { }


	bool load() {
		_buffer = Asset::read(path);
		return _buffer != nullptr;
	}

	const std::string path;
private:
	friend class SoundManager;
	bytes _buffer = nullptr;
};

class SoundManager {
public:
	SoundManager() { }
	bool start() {
		SLresult lRes;
		const SLuint32 lEngineMixIIDCount = 1;
		const SLInterfaceID lEngineMixIIDs[] = { SL_IID_ENGINE };
		const SLboolean lEngineMixReqs[] = { SL_BOOLEAN_TRUE };
		const SLuint32 lOutputMixIIDCount = 0;
		const SLInterfaceID lOutputMixIIDs[] = {};
		const SLboolean lOutputMixReqs[] = {};

		lRes = slCreateEngine(&_engine_obj, 0, NULL, lEngineMixIIDCount, lEngineMixIIDs, lEngineMixReqs);
		if (lRes != SL_RESULT_SUCCESS) return stop(), false;

		lRes = (*_engine_obj)->Realize(_engine_obj, SL_BOOLEAN_FALSE);
		if (lRes != SL_RESULT_SUCCESS) return stop(), false;

		lRes = (*_engine_obj)->GetInterface(_engine_obj, SL_IID_ENGINE, &_engine);
		if (lRes != SL_RESULT_SUCCESS) return stop(), false;

		lRes = (*_engine)->CreateOutputMix(_engine, &_output_mix_obj, lOutputMixIIDCount, lOutputMixIIDs, lOutputMixReqs);

		lRes = (*_output_mix_obj)->Realize(_output_mix_obj, SL_BOOLEAN_FALSE);


		if (!startSoundPlayer()) return stop(), false;
		for (int32_t i = 0; i < _sounds.size(); ++i)
			if (!_sounds[i]->load()) return stop(), false;


		return true;
	}
	void stop() {
		stopBackground();
		if (_output_mix_obj) {
			(*_output_mix_obj)->Destroy(_output_mix_obj);
			_output_mix_obj = nullptr;
		}
		if (_engine_obj) {
			(*_engine_obj)->Destroy(_engine_obj);
			_engine_obj = nullptr;
			_engine = nullptr;
		}

		if (_player_obj) {
			(*_player_obj)->Destroy(_player_obj);
			_player_obj = nullptr;
			_player = nullptr;
			_player_queue = nullptr;
			_player_volume = nullptr;
		}
		for (int i = 0; i < _sounds.size(); ++i) {
			delete _sounds[i];
		}
		_sounds.clear();
	}

	~SoundManager() {
		stop();
	}

	bool playBackground(const std::string& name) {
		SLresult lRes;

		Asset asset(name);
		AssetDescriptor asset_descriptor = asset.descript();
		if (asset_descriptor.descriptor < 0) {
			LOGE("Could not open BGM file: %s", name.c_str());
			return false;
		}
		asset.close();

		SLDataLocator_AndroidFD lDataLocatorIn;
		lDataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDFD;
		lDataLocatorIn.fd = asset_descriptor.descriptor;
		lDataLocatorIn.offset = asset_descriptor.start;
		lDataLocatorIn.length = asset_descriptor.end;

		SLDataFormat_MIME lDataFormat;
		lDataFormat.formatType = SL_DATAFORMAT_MIME;
		lDataFormat.mimeType = NULL;
		lDataFormat.containerType = SL_CONTAINERTYPE_UNSPECIFIED;

		SLDataSource lDataSource;
		lDataSource.pLocator = &lDataLocatorIn;
		lDataSource.pFormat = &lDataFormat;

		SLDataLocator_OutputMix lDataLocatorOut;
		lDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		lDataLocatorOut.outputMix = _output_mix_obj;

		SLDataSink lDataSink;
		lDataSink.pLocator = &lDataLocatorOut;
		lDataSink.pFormat = NULL;

		const SLuint32 lBGMPlayerIIDCount = 3;
		const SLInterfaceID lBGMPlayerIIDs[] = { SL_IID_PLAY, SL_IID_SEEK, SL_IID_VOLUME };
		const SLboolean lBGMPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

		lRes = (*_engine)->CreateAudioPlayer(_engine,
			&_bgm_play_obj, &lDataSource, &lDataSink,
			lBGMPlayerIIDCount, lBGMPlayerIIDs, lBGMPlayerReqs);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_bgm_play_obj)->Realize(_bgm_play_obj, SL_BOOLEAN_FALSE);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_bgm_play_obj)->GetInterface(_bgm_play_obj, SL_IID_PLAY, &_bgm_player);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_bgm_play_obj)->GetInterface(_bgm_play_obj, SL_IID_SEEK, &_bgm_player_seek);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_bgm_play_obj)->GetInterface(_bgm_play_obj, SL_IID_VOLUME, &_bgm_player_volume);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_bgm_player_seek)->SetLoop(_bgm_player_seek, SL_BOOLEAN_TRUE, 0 /*play from start*/, SL_TIME_UNKNOWN /*to the end*/);
		if (lRes != SL_RESULT_SUCCESS) return false;

		// repead music play
		lRes = (*_bgm_player)->SetPlayState(_bgm_player, SL_PLAYSTATE_PLAYING);
		if (lRes != SL_RESULT_SUCCESS) return false;

		return true;
	}
	void stopBackground() {
		if (_bgm_player) {

			SLuint32 lBGMPlayerState;
			(*_bgm_play_obj)->GetState(_bgm_play_obj, &lBGMPlayerState);

			if (lBGMPlayerState == SL_OBJECT_STATE_REALIZED) {
				(*_bgm_player)->SetPlayState(_bgm_player, SL_PLAYSTATE_PAUSED);
				(*_bgm_play_obj)->Destroy(_bgm_play_obj);

				_bgm_play_obj = nullptr;
				_bgm_player = nullptr;
				_bgm_player_seek = nullptr;
				_bgm_player_volume = nullptr;
			}
		}
	}

	Sound* registerSound(const std::string& path) {
		for (int32_t i = 0; i < _sounds.size(); ++i)
			if (path == _sounds[i]->path)
				return _sounds[i];
		Sound* lSound = new Sound(path);
		_sounds.push_back(lSound);
		return lSound;
	}
	bool playSound(const std::string& path) {
		for (auto& sound : _sounds)
			if (sound->path == path)
				playSound(sound);
		LOGI("SOUND %s NOT REGISTERED", path.c_str());
		return false;
	}
	bool playSound(Sound* pSound) {
		SLresult lRes;
		SLuint32 lPlayerState;
		(*_player_obj)->GetState(_player_obj, &lPlayerState);
		if (lPlayerState == SL_OBJECT_STATE_REALIZED) {
			int16_t* lBuffer = (int16_t*)pSound->_buffer.get();
			off_t lLength = pSound->_buffer.lenght;

			// Removes any sound from the queue.
			lRes = (*_player_queue)->Clear(_player_queue);
			if (lRes != SL_RESULT_SUCCESS) return false;

			// Plays the new sound.
			lRes = (*_player_queue)->Enqueue(_player_queue, lBuffer, lLength);
			if (lRes != SL_RESULT_SUCCESS) return false;
		}
		return true;
	}
private:
	bool startSoundPlayer() {
		SLresult lRes;
		// Set-up sound audio source.
		SLDataLocator_AndroidSimpleBufferQueue lDataLocatorIn;
		lDataLocatorIn.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
		// At most one buffer in the queue.
		lDataLocatorIn.numBuffers = 1;


		//SLDataFormat_MIME lDataFormat = { SL_DATAFORMAT_MIME,NULL, SL_CONTAINERTYPE_UNSPECIFIED };

		SLDataFormat_PCM lDataFormat;
		lDataFormat.formatType = SL_DATAFORMAT_PCM; // SL_DATAFORMAT_MIME;
		lDataFormat.numChannels = 1;
		lDataFormat.samplesPerSec = SL_SAMPLINGRATE_44_1;
		lDataFormat.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
		lDataFormat.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
		lDataFormat.channelMask = lDataFormat.numChannels == 1 ? SL_SPEAKER_FRONT_CENTER : (SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT);
		lDataFormat.endianness = SL_BYTEORDER_LITTLEENDIAN;


		SLDataSource lDataSource;
		lDataSource.pLocator = &lDataLocatorIn;
		lDataSource.pFormat = &lDataFormat;

		SLDataLocator_OutputMix lDataLocatorOut;
		lDataLocatorOut.locatorType = SL_DATALOCATOR_OUTPUTMIX;
		lDataLocatorOut.outputMix = _output_mix_obj;

		SLDataSink lDataSink;
		lDataSink.pLocator = &lDataLocatorOut;
		lDataSink.pFormat = NULL;

		const SLuint32 lSoundPlayerIIDCount = 3;
		const SLInterfaceID lSoundPlayerIIDs[] = { SL_IID_PLAY, SL_IID_BUFFERQUEUE,SL_IID_VOLUME };
		const SLboolean lSoundPlayerReqs[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };

		lRes = (*_engine)->CreateAudioPlayer(_engine, &_player_obj,
			&lDataSource, &lDataSink, lSoundPlayerIIDCount,
			lSoundPlayerIIDs, lSoundPlayerReqs);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_player_obj)->Realize(_player_obj, SL_BOOLEAN_FALSE);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_player_obj)->GetInterface(_player_obj, SL_IID_PLAY, &_player);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_player_obj)->GetInterface(_player_obj, SL_IID_BUFFERQUEUE, &_player_queue);
		if (lRes != SL_RESULT_SUCCESS) return false;

		lRes = (*_player_obj)->GetInterface(_player_obj, SL_IID_VOLUME, &_player_volume);

		if (lRes != SL_RESULT_SUCCESS) return false;

		// set queue to ready state
		lRes = (*_player)->SetPlayState(_player, SL_PLAYSTATE_PLAYING);
		if (lRes != SL_RESULT_SUCCESS) return false;

		return true;
	}
private:
	SLObjectItf _engine_obj = nullptr;
	SLEngineItf _engine = nullptr;
	SLObjectItf _output_mix_obj = nullptr;

	SLObjectItf _bgm_play_obj = nullptr;
	SLPlayItf _bgm_player = nullptr;
	SLSeekItf _bgm_player_seek = nullptr;
	SLVolumeItf _bgm_player_volume = nullptr;

	SLObjectItf _player_obj = nullptr;
	SLPlayItf _player = nullptr;
	SLBufferQueueItf _player_queue = nullptr;
	SLVolumeItf _player_volume = nullptr;
	std::vector<Sound*> _sounds;
};
