// Copyright 2019 Samsung Electronics Inc. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/media/castanets/castanets_renderer_media_player_manager.h"
#include "content/common/media/castanets_media_player_messages.h"
#include "content/common/media/castanets_media_player_messages_enums.h"

#include "content/renderer/media/castanets/castanets_webmediaplayer_impl.h"
#include "content/common/media/castanets_media_player_init_config.h"
#include "content/public/renderer/render_thread.h"
#include "content/renderer/render_thread_impl.h"
#include "content/renderer/renderer_blink_platform_impl.h"
#include "media/base/bind_to_current_loop.h"
#include "base/bind.h"
#include "base/message_loop/message_loop.h"

#if defined(TIZEN_VIDEO_HOLE)
#include "content/public/renderer/render_frame.h"
#include "content/public/renderer/render_view.h"
#include "third_party/blink/public/web/web_view.h"
#endif

namespace content {

CastanetsRendererMediaPlayerManager::CastanetsRendererMediaPlayerManager(
    RenderFrame* render_frame)
    : RenderFrameObserver(render_frame) {
    LOG(INFO) << __FUNCTION__;
    }

CastanetsRendererMediaPlayerManager::~CastanetsRendererMediaPlayerManager() {
  DCHECK(media_players_.empty())
      << "CastanetsRendererMediaPlayerManager is owned by RenderFrameImpl and "
         "is "
         "destroyed only after all media players are destroyed.";
}

int CastanetsRendererMediaPlayerManager::RegisterMediaPlayer(
    blink::WebMediaPlayer* player) {
  // Note : For the unique player id among the all renderer process,
  // generate player id based on renderer process id.
  LOG(INFO) << __FUNCTION__;
  static int next_media_player_id_ = base::GetCurrentProcId() << 16;
  next_media_player_id_ = (next_media_player_id_ & 0xFFFF0000) |
                          ((next_media_player_id_ + 1) & 0x0000FFFF);
  media_players_[next_media_player_id_] = player;
  return next_media_player_id_;
}

void CastanetsRendererMediaPlayerManager::UnregisterMediaPlayer(int player_id) {
  LOG(INFO) << __FUNCTION__;
  media_players_.erase(player_id);
}

blink::WebMediaPlayer*
CastanetsRendererMediaPlayerManager::GetMediaPlayer(int player_id) {
  LOG(INFO) << __FUNCTION__;
  std::map<int, blink::WebMediaPlayer*>::iterator iter =
      media_players_.find(player_id);
  if (iter != media_players_.end())
    return iter->second;
  return NULL;
}

void CastanetsRendererMediaPlayerManager::OnDestruct() {
  LOG(INFO) << __FUNCTION__;
  delete this;
}

bool CastanetsRendererMediaPlayerManager::OnMessageReceived(
    const IPC::Message& message) {
  LOG(INFO) << __FUNCTION__;
  bool handled = true;
  IPC_BEGIN_MESSAGE_MAP(CastanetsRendererMediaPlayerManager, message)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_MediaDataChanged, OnMediaDataChange)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_DurationChanged, OnDurationChanged)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_TimeUpdate, OnTimeUpdate)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_BufferUpdate, OnBufferUpdate)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_ReadyStateChange, OnReadyStateChange)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_NetworkStateChange,
                      OnNetworkStateChange)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_TimeChanged, OnTimeChanged)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_PauseStateChanged, OnPauseStateChange)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_OnSeekComplete, OnSeekComplete)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_SeekRequest, OnRequestSeek)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_PlayerSuspend, OnPlayerSuspend)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_PlayerResumed, OnPlayerResumed)
  IPC_MESSAGE_HANDLER(MediaPlayerMsg_PlayerDestroyed, OnPlayerDestroyed)
  IPC_MESSAGE_UNHANDLED(handled = false)
  IPC_END_MESSAGE_MAP()
  return handled;
}

void CastanetsRendererMediaPlayerManager::Initialize(
    int player_id,
    MediaPlayerHostMsg_Initialize_Type type,
    const GURL& url,
    const std::string& mime_type,
    int demuxer_client_id) {
  LOG(INFO) << __FUNCTION__;
  bool has_encrypted_listener_or_cdm = false;
  MediaPlayerInitConfig config{type, url, mime_type, demuxer_client_id,
                               has_encrypted_listener_or_cdm, 0 , 10};
  LOG(INFO) << "render side routing id: "<< routing_id();
  //Send(new MediaPlayerHostMsg_DeInit(routing_id(), -1));  
  LOG(INFO) << "renderer config: "<< config.type << " " << config.url << " "<<config.mime_type <<" "<< config.demuxer_client_id <<" "<< config.has_encrypted_listener_or_cdm;
  int x = Send(new MediaPlayerHostMsg_Init(routing_id(), player_id, config));	
  LOG(INFO) << "IPC return: "<< x;
}

void CastanetsRendererMediaPlayerManager::OnMediaDataChange(int player_id,
                                                            int width,
                                                            int height,
                                                            int media) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnMediaDataChange(width, height, media);
}

void CastanetsRendererMediaPlayerManager::OnPlayerDestroyed(int player_id) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnPlayerDestroyed();
}

void CastanetsRendererMediaPlayerManager::OnDurationChanged(
    int player_id,
    base::TimeDelta duration) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnDurationChanged(duration);
}

void CastanetsRendererMediaPlayerManager::OnTimeUpdate(
    int player_id,
    base::TimeDelta current_time) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnTimeUpdate(current_time);
}

void CastanetsRendererMediaPlayerManager::OnBufferUpdate(int player_id,
                                                         int percentage) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnBufferUpdate(percentage);
}

void CastanetsRendererMediaPlayerManager::OnReadyStateChange(
    int player_id,
    blink::WebMediaPlayer::ReadyState state) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->SetReadyState(
        static_cast<blink::WebMediaPlayer::ReadyState>(state));
}

void CastanetsRendererMediaPlayerManager::OnNetworkStateChange(
    int player_id,
    blink::WebMediaPlayer::NetworkState state) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->SetNetworkState(
        static_cast<blink::WebMediaPlayer::NetworkState>(state));
}

void CastanetsRendererMediaPlayerManager::OnTimeChanged(int player_id) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnTimeChanged();
}

void CastanetsRendererMediaPlayerManager::OnSeekComplete(int player_id) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnSeekComplete();
}

void CastanetsRendererMediaPlayerManager::OnPauseStateChange(int player_id,
                                                             bool state) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnPauseStateChange(state);
}

void CastanetsRendererMediaPlayerManager::OnRequestSeek(
    int player_id,
    base::TimeDelta seek_time) {
  LOG(INFO) << __FUNCTION__;
  blink::WebMediaPlayer* player = GetMediaPlayer(player_id);
  if (player)
    player->OnRequestSeek(seek_time);
}

void CastanetsRendererMediaPlayerManager::OnPlayerSuspend(int player_id,
                                                          bool is_preempted) {
  LOG(INFO) << __FUNCTION__;
  if (blink::WebMediaPlayer* player = GetMediaPlayer(player_id))
    player->OnPlayerSuspend(is_preempted);
}

void CastanetsRendererMediaPlayerManager::OnPlayerResumed(int player_id,
                                                          bool is_preempted) {
  LOG(INFO) << __FUNCTION__;
  if (blink::WebMediaPlayer* player = GetMediaPlayer(player_id))
    player->OnPlayerResumed(is_preempted);
}

void CastanetsRendererMediaPlayerManager::Start(int player_id) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_Play(routing_id(), player_id));
}

void CastanetsRendererMediaPlayerManager::Pause(int player_id,
                                                bool is_media_related_action) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_Pause(routing_id(), player_id,
                                       is_media_related_action));
}


void CastanetsRendererMediaPlayerManager::Suspend(int player_id) {
  LOG(INFO) << "player_id : " << player_id;
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_Suspend(routing_id(), player_id));
}

void CastanetsRendererMediaPlayerManager::Resume(int player_id) {
  LOG(INFO) << "player_id : " << player_id;
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_Resume(routing_id(), player_id));
}


void CastanetsRendererMediaPlayerManager::Seek(int player_id,
                                               base::TimeDelta time) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_Seek(routing_id(), player_id, time));
}

void CastanetsRendererMediaPlayerManager::SetVolume(int player_id,
                                                    double volume) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_SetVolume(routing_id(), player_id, volume));
}

void CastanetsRendererMediaPlayerManager::SetRate(int player_id, double rate) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_SetRate(routing_id(), player_id, rate));
}

void CastanetsRendererMediaPlayerManager::DestroyPlayer(int player_id) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_DeInit(routing_id(), player_id));
}

void CastanetsRendererMediaPlayerManager::EnteredFullscreen(int player_id) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_EnteredFullscreen(routing_id(), player_id));
}

void CastanetsRendererMediaPlayerManager::ExitedFullscreen(int player_id) {
  LOG(INFO) << __FUNCTION__;
  Send(new MediaPlayerHostMsg_ExitedFullscreen(routing_id(), player_id));
}

void CastanetsRendererMediaPlayerManager::SetMediaGeometry(
    int player_id,
    const gfx::RectF& rect) {
  LOG(INFO) << __FUNCTION__;
  gfx::RectF video_rect = rect;
  Send(new MediaPlayerHostMsg_SetGeometry(routing_id(), player_id,
                                             video_rect));
}

}  // namespace content
