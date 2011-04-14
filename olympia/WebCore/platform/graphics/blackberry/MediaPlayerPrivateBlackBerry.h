/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef MediaPlayerPrivateOlympia_h
#define MediaPlayerPrivateOlympia_h

#if ENABLE(VIDEO)

#include "MediaPlayerPrivate.h"
#include "Timer.h"
#include <wtf/OwnPtr.h>

namespace WebCore {

    class GraphicsContext;
    class IntSize;
    class IntRect;
    class String;

    class MediaPlayerPrivate : public MediaPlayerPrivateInterface {
    public:
        static void registerMediaEngine(MediaEngineRegistrar);

        ~MediaPlayerPrivate();

        // MediaPlayerPrivateInterface implementation
        IntSize naturalSize() const;
        bool hasVideo() const;
        bool hasAudio() const;

        void load(const String& url);
        void cancelLoad();

        void prepareToPlay();
        void play();
        void pause();

        bool paused() const;
        bool seeking() const;

        float duration() const;
        float currentTime() const;
        void seek(float time);
        void setEndTime(float);

        void setRate(float);
        void setVolume(float);

        int dataRate() const;

        MediaPlayer::NetworkState networkState() const { return m_networkState; }
        MediaPlayer::ReadyState readyState() const { return m_readyState; }

        PassRefPtr<TimeRanges> buffered() const;
        float maxTimeSeekable() const;
        unsigned bytesLoaded() const;
        bool totalBytesKnown() const;
        unsigned totalBytes() const;

        void setVisible(bool);
        void setSize(const IntSize&);

        void loadStateChanged();
        void didEnd();

        void paint(GraphicsContext*, const IntRect&);

    private:
        MediaPlayerPrivate(MediaPlayer*);

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
        void setMediaPlayerProxy(WebMediaPlayerProxy*);
        void setPoster(const String& url);
        void deliverNotification(MediaPlayerProxyNotificationType);
#endif

        // Engine support
        static MediaPlayerPrivateInterface* create(MediaPlayer*);
        static void getSupportedTypes(HashSet<String>& types);
        static MediaPlayer::SupportsType supportsType(const String& type, const String& codecs);
        static bool isAvailable();

        MediaPlayer* m_player;
        MediaPlayer::NetworkState m_networkState;
        MediaPlayer::ReadyState m_readyState;
        bool m_startedPlaying;
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
        WebMediaPlayerProxy* m_proxy;
#endif
    };
}

#endif

#endif
