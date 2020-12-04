// Copyright 2019 Samsung Electronics Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_RENDERER_MEDIA_PLAYER_MANAGER_H_
#define CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_RENDERER_MEDIA_PLAYER_MANAGER_H_

#include <map>
#include <vector>

#include "content/public/renderer/render_frame_observer.h"
#include "content/common/media/castanets_media_player_messages_enums.h"
#include "third_party/blink/public/platform/web_media_player.h"
#include "ui/gfx/geometry/rect_f.h"

#include "base/memory/shared_memory.h"
#include "content/public/renderer/render_frame_observer.h"
#include "media/base/demuxer_stream.h"
#include "url/gurl.h"

#if defined(TIZEN_TBM_SUPPORT)
#include "ui/gfx/tbm_buffer_handle.h"
#endif


namespace blink {
enum class WebRemotePlaybackAvailability;
}

namespace content {
class WebMediaPlayerEfl;

class CastanetsRendererMediaPlayerManager
    : public RenderFrameObserver {
 public:
  // Constructs a CastanetsRendererMediaPlayerManager object for the
  // |render_frame|.
  explicit CastanetsRendererMediaPlayerManager(RenderFrame* render_frame);
  ~CastanetsRendererMediaPlayerManager() override;

  void Initialize(int player_id,
                  MediaPlayerHostMsg_Initialize_Type type,
                  const GURL& url,
                  const std::string& mime_type,
                  int demuxer_client_id);
  void Start(int player_id);

  // Pauses the player.
  // is_media_related_action should be true if this pause is coming from an
  // an action that explicitly pauses the video (user pressing pause, JS, etc.)
  // Otherwise it should be false if Pause is being called due to other reasons
  // (cleanup, freeing resources, etc.)
  void Pause(int player_id, bool is_media_related_action);
  void Seek(int player_id, base::TimeDelta time);
  void SetVolume(int player_id, double volume);
  void SetRate(int player_id, double rate);

  // Registers and unregisters a WebMediaPlayerEfl object.
  int RegisterMediaPlayer(blink::WebMediaPlayer* player);
  void UnregisterMediaPlayer(int player_id);

  void DestroyPlayer(int player_id);
  void Suspend(int player_id);
  void Resume(int player_id);
  void Activate(int player_id);
  void Deactivate(int player_id);

  // Requests the player to enter/exit fullscreen.
  void EnteredFullscreen(int player_id);
  void ExitedFullscreen(int player_id);

  void SetMediaGeometry(int player_id, const gfx::RectF& rect);

  void OnDestruct() override;
  bool OnMessageReceived(const IPC::Message& message) override;

  private:
  void OnMediaDataChange(int player_id, int width, int height, int media);
  void OnPlayerDestroyed(int player_id);
  void OnDurationChanged(int player_id, base::TimeDelta duration);
  void OnTimeUpdate(int player_id, base::TimeDelta current_time);
  void OnBufferUpdate(int player_id, int percentage);
  void OnReadyStateChange(int player_id,
                          blink::WebMediaPlayer::ReadyState state);
  void OnNetworkStateChange(int player_id,
                            blink::WebMediaPlayer::NetworkState state);
  void OnTimeChanged(int player_id);
  void OnSeekComplete(int player_id);
  void OnPauseStateChange(int player_id, bool state);
  void OnRequestSeek(int player_id, base::TimeDelta seek_time);
  void OnPlayerSuspend(int player_id, bool is_preempted);
  void OnPlayerResumed(int player_id, bool is_preempted);
 
  blink::WebMediaPlayer* GetMediaPlayer(int player_id);

  std::map<int, blink::WebMediaPlayer*> media_players_;

  DISALLOW_COPY_AND_ASSIGN(CastanetsRendererMediaPlayerManager);
};

}  // namespace content

#endif  // CONTENT_RENDERER_MEDIA_CASTANETS_CASTANETS_RENDERER_MEDIA_PLAYER_MANAGER_H_
