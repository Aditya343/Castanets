// Copyright 2019 Samsung Electronics Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_WEBMEDIAPLAYER_IMPL_H_
#define CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_WEBMEDIAPLAYER_IMPL_H_

#include <string>
#include <utility>
#include <vector>

#include "media/blink/video_frame_compositor.h"
#include "media/renderers/paint_canvas_video_renderer.h"
//#include "third_party/blink/public/platform/web_media_player.h"
#include "media/blink/webmediaplayer_delegate.h"
#include "content/renderer/media/castanets/castanets_renderer_media_player_manager.h"

#include "base/memory/shared_memory.h"
#include "base/message_loop/message_loop.h"
#include "base/trace_event/memory_dump_provider.h"
#include "cc/layers/video_frame_provider_client_impl.h"
#include "media/base/cdm_context.h"
#include "media/base/video_frame.h"
#include "media/base/waiting.h"
#include "media/blink/webmediaplayer_params.h"
#include "media/blink/webmediaplayer_util.h"
#include "third_party/blink/public/platform/web_audio_source_provider.h"
#include "third_party/blink/public/platform/web_media_player.h"
#if defined(OS_TIZEN_TV_PRODUCT)
#define USE_NO_WAIT_SIGNAL_MEDIA_PACKET_THREAD 1
#endif

namespace blink {
class WebLocalFrame;
class WebMediaPlayerClient;
class WebMediaPlayerEncryptedMediaClient;
}  // namespace blink

namespace base {
class SingleThreadTaskRunner;
}  // namespace base

namespace cc {
class VideoLayer;
}

namespace media {
class MediaLog;
class UrlIndex;
class WebAudioSourceProviderImpl;
class WebMediaPlayerDelegate;
}  // namespace media

namespace content {
class CastanetsRendererMediaPlayerManager;

// This class implements blink::WebMediaPlayer by keeping the castanets
// media player in the browser process. It listens to all the status changes
// sent from the browser process and sends playback controls to the media
// player.
class WebMediaPlayerCastanets
    : public blink::WebMediaPlayer,
      public media::WebMediaPlayerDelegate::Observer,
      public base::SupportsWeakPtr<WebMediaPlayerCastanets> {
 public:
  // Construct a WebMediaPlayerCastanets object. This class communicates
  // with the WebMediaPlayerCastanets object in the browser process through
  // |proxy|.
  WebMediaPlayerCastanets(
      blink::WebLocalFrame* frame,
      blink::WebMediaPlayerClient* client,
      blink::WebMediaPlayerEncryptedMediaClient* encrypted_client,
      media::WebMediaPlayerDelegate* delegate,
      media::UrlIndex* url_index,
      std::unique_ptr<media::VideoFrameCompositor> compositor,
      std::unique_ptr<media::WebMediaPlayerParams> params);
  ~WebMediaPlayerCastanets() override;

  LoadTiming Load(LoadType,
                  const blink::WebMediaPlayerSource&,
                  CorsMode) override;

  // Playback controls.
  void Play() override;
  void Pause() override;
  void Seek(double seconds) override;
  void SetRate(double rate) override;
  void SetVolume(double volume) override;

  blink::WebTimeRanges Buffered() const override;
  blink::WebTimeRanges Seekable() const override;

  // True if the loaded media has a playable video/audio track.
  bool HasVideo() const override;
  bool HasAudio() const override;

  // Dimension of the video.
  blink::WebSize NaturalSize() const override;

  blink::WebSize VisibleRect() const override;

  // Getters of playback state.
  bool Paused() const override;
  bool Seeking() const override;
  double Duration() const override;
  double CurrentTime() const override;

  // Internal states of loading and network.
  blink::WebMediaPlayer::NetworkState GetNetworkState() const override;
  blink::WebMediaPlayer::ReadyState GetReadyState() const override;

  // Returns an implementation-specific human readable error message, or an
  // empty string if no message is available. The message should begin with a
  // UA-specific-error-code (without any ':'), optionally followed by ': ' and
  // further description of the error.
  blink::WebString GetErrorMessage() const override;

  bool DidLoadingProgress() override;

  double MediaTimeForTimeValue(double time_value) const override;

  // |out_metadata|, if set, is used to return metadata about the frame that is
  // uploaded during this call. |already_uploaded_id| indicates the unique_id of
  // the frame last uploaded to this destination. It should only be set by the
  // caller if the contents of the destination are known not to have changed
  // since that upload. - If |out_metadata| is not null, |already_uploaded_id|
  // is compared with the unique_id of the frame being uploaded. If it's the
  // same, the upload may be skipped and considered to be successful.

  void SetContentDecryptionModule(
      blink::WebContentDecryptionModule* cdm,
      blink::WebContentDecryptionModuleResult result) override;

  void EnteredFullscreen() override;
  void ExitedFullscreen() override;

  // WebMediaPlayerDelegate::Observer implementation.
  void OnFrameHidden() override;
  void OnFrameShown() override;
  void OnPlay() override;
  void OnPause() override;
  void OnSeekForward(double seconds) override;
  void OnSeekBackward(double seconds) override;
  void OnVolumeMultiplierUpdate(double multiplier) override;
  void OnBecamePersistentVideo(bool value) override;

  void OnMediaDataChange(int, int, int) override;
  void OnDurationChanged(base::TimeDelta Duration) override;
  void OnTimeChanged() override;
  void OnTimeUpdate(base::TimeDelta) override;
  void OnBufferUpdate(int) override;
  void OnPauseStateChange(bool) override;

  void OnRequestSeek(base::TimeDelta seek_time) override;

  // Internal seeks can happen. So don't include time as argument.
  void OnSeekComplete() override;

  void OnPlayerSuspend(bool) override;
  void OnPlayerResumed(bool) override;
  void OnPlayerDestroyed() override;

  void SetReadyState(WebMediaPlayer::ReadyState) override;
  void SetNetworkState(WebMediaPlayer::NetworkState) override;

  void SuspendAndReleaseResources();

  void SetMediaPlayerManager(
      content::CastanetsRendererMediaPlayerManager* media_player_manager);

  void RequestPause() override;
  void ReleaseMediaResource();
  void InitializeMediaResource();

  void CreateVideoHoleFrame();
  void OnDrawableContentRectChanged(gfx::Rect rect, bool is_video);

  void StartLayerBoundUpdateTimer();
  void StopLayerBoundUpdateTimer();
  void OnLayerBoundUpdateTimerFired();

  void OnRequestPictureInPicture() override;
  void SetSinkId(const blink::WebString& sing_id,
                 blink::WebSetSinkIdCompleteCallback) override;
  blink::WebMediaPlayer::SurfaceLayerMode GetVideoSurfaceLayerMode()
      const override;
  bool WouldTaintOrigin() const override;
  unsigned DecodedFrameCount() const override;
  unsigned DroppedFrameCount() const override;
  uint64_t AudioDecodedByteCount() const override;
  uint64_t VideoDecodedByteCount() const override;
  void Paint(cc::PaintCanvas*,
             const blink::WebRect&,
             cc::PaintFlags&,
             int already_uploaded_id = -1,
             VideoFrameUploadMetadata* out_metadata = nullptr) override;
  base::WeakPtr<WebMediaPlayer> AsWeakPtr() override;
  void OnFrameClosed() override;
  void OnIdleTimeout() override;
  void OnMuted(bool muted) override;
 
private:
  // Called after |defer_load_cb_| has decided to allow the load. If
  // |defer_load_cb_| is null this is called immediately.
  void DoLoad(LoadType load_type, const blink::WebURL& url);
  void PauseInternal(bool is_media_related_action);

  void OnNaturalSizeChanged(gfx::Size size);
  void OnOpacityChanged(bool opaque);

  // Returns the current video frame from |compositor_|. Blocks until the
  // compositor can return the frame.
  scoped_refptr<media::VideoFrame> GetCurrentFrameFromCompositor() const;

  // Called whenever there is new frame to be painted.
  void FrameReady(const scoped_refptr<media::VideoFrame>& frame);

  // Calculate the boundary rectangle of the media player (i.e. location and
  // size of the video frame).
  // Returns true if the geometry has been changed since the last call.
  bool UpdateBoundaryRectangle();
  const gfx::RectF GetBoundaryRectangle();

  // TODO: Fix the scope!
  void Resume();

  blink::WebLocalFrame* frame_;

  blink::WebMediaPlayer::NetworkState network_state_;
  blink::WebMediaPlayer::ReadyState ready_state_;

  // Message loops for posting tasks on Chrome's main thread. Also used
  // for DCHECKs so methods calls won't execute in the wrong thread.
  const scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;

  // Manager for managing this object and for delegating method calls on
  // Render Thread.
  content::CastanetsRendererMediaPlayerManager* manager_;

  blink::WebMediaPlayerClient* client_;

  std::unique_ptr<media::MediaLog> media_log_;

  media::WebMediaPlayerDelegate* const delegate_;
  int delegate_id_;

  media::WebMediaPlayerParams::DeferLoadCB defer_load_cb_;

  // Video rendering members.
  // The |compositor_| runs on the compositor thread, or if
  // kEnableSurfaceLayerForVideo is enabled, the media thread. This task runner
  // posts tasks for the |compositor_| on the correct thread.
  scoped_refptr<base::SingleThreadTaskRunner> compositor_task_runner_;

  // Deleted on |compositor_task_runner_|.
  std::unique_ptr<media::VideoFrameCompositor> compositor_;

  // The compositor layer for displaying the video content when using composited
  // playback.
  scoped_refptr<cc::VideoLayer> video_layer_;

  MediaPlayerHostMsg_Initialize_Type player_type_;

  // Player ID assigned by the |manager_|.
  int player_id_;

  int video_width_;
  int video_height_;

  bool audio_;
  bool video_;

  base::TimeDelta current_time_;
  base::TimeDelta duration_;
  bool is_paused_;

  bool is_seeking_;
  base::TimeDelta seek_time_;
  bool pending_seek_;
  base::TimeDelta pending_seek_time_;

  // Whether the video is known to be opaque or not.
  bool opaque_;
  bool is_fullscreen_;

  bool is_draw_ready_;
  bool pending_play_;

  // A rectangle represents the geometry of video frame, when computed last
  // time.
  gfx::RectF last_computed_rect_;
  base::RepeatingTimer layer_bound_update_timer_;

  gfx::Size natural_size_;
  blink::WebTimeRanges buffered_;
  mutable bool did_loading_progress_;

  // The last volume received by setVolume() and the last volume multiplier from
  // OnVolumeMultiplierUpdate(). The multiplier is typical 1.0, but may be less
  // if the WebMediaPlayerDelegate has requested a volume reduction (ducking)
  // for a transient sound.  Playout volume is derived by volume * multiplier.
  double volume_;

  base::WeakPtr<WebMediaPlayerCastanets> weak_this_;
  base::WeakPtrFactory<WebMediaPlayerCastanets> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(WebMediaPlayerCastanets);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_WEBMEDIAPLAYER_IMPL_H_
