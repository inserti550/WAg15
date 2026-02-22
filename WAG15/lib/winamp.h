#pragma once
#include "gen.h"
#include "wa_ipc.h"
#include <windows.h>


extern winampGeneralPurposePlugin plugin;

namespace Winamp {

inline void (*export_sa_setreq)(int want) = NULL;
inline char *(*export_sa_get)(char data[75 * 2 + 8]) = NULL;
inline bool sa_inited = false;

inline void InitSA() {
  if (!sa_inited) {
    export_sa_setreq =
        (void (*)(int))SendMessage(plugin.hwndParent, WM_WA_IPC, 1, 800);

    export_sa_get =
        (char *(*)(char[158]))SendMessage(plugin.hwndParent, WM_WA_IPC, 2, 800);

    if (export_sa_setreq) {
      export_sa_setreq(1);
      sa_inited = true;
    }
  }
}

inline int GetStatus() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)0,
                          IPC_ISPLAYING);
}

// previous track
inline void PreviousTrack() {
  SendMessage(plugin.hwndParent, WM_COMMAND, 40044, 0);
}

// next track
inline void NextTrack() {
  SendMessage(plugin.hwndParent, WM_COMMAND, 40048, 0);
}

// play this track
inline void Play() { SendMessage(plugin.hwndParent, WM_COMMAND, 40045, 0); }

// stop this track
inline void Stop() { SendMessage(plugin.hwndParent, WM_COMMAND, 40047, 0); }

// pause this track
inline void Pause() { SendMessage(plugin.hwndParent, WM_COMMAND, 40046, 0); }

inline int GetBitrate() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 1, IPC_GETINFO);
}

inline int GetSampleRate() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETINFO);
}

inline int GetChannels() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 2, IPC_GETINFO);
}

/* return int
** If it returns 1, Winamp is playing.
** If it returns 3, Winamp is paused.
** If it returns 0, Winamp is not playing.
*/
inline int IsPlay() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING);
}
/* range 0-255
If you pass 'volume' as -666 then the message will return the current volume.*/
inline int SetVolume(int Volume) {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, Volume, IPC_SETVOLUME);
}
/* return volume on range 0-255 */
inline int GetVolume() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, (WPARAM)-666,
                          IPC_SETVOLUME);
}
/* return playlist position */
inline int GetListPosition() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
}
/* (requires Winamp 5.61+)
** int pos=SendMessage(hwnd_winamp,WM_WA_IPC,0,IPC_GETNEXTLISTPOS);
** IPC_GETNEXTLISTPOS returns the next playlist position expected to be played
* from the
** current playlist and allows for determining the next playlist item to be
* played even
** if shuffle mode (see IPC_GET_SHUFFLE) is enabled at the time of using this
* api.
**
** If there is no known next playlist item then this will return -1 i.e. if
* there's only
** one playlist item or at the end of the current playlist and repeat is
* disabled.
**
** Notes: If a plug-in (like the JTFE plug-in) uses IPC_GET_NEXT_PLITEM to
* override the
**        playlist order then you will need to query the plug-in directly (via
* api_queue
**        for the JTFE plug-in) to get the correct next playlist item to be
* played.
**
**        If a change is made to the internal shuffle table, the value returned
* by prior
**        use of this api is likely to be different and so will need to be
* re-queried.
**
**        The returned playlist item position is zero-based.
*/
inline int GetNextListPosition() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS);
}
/* return 1 when enable 0 when disable
 */
inline int GetTrackLengthMs(int index) {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, index, 211);
}
inline int GetPlaylistLength() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GETLISTLENGTH);
}
inline bool GetShuffle() {
  return (bool)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_SHUFFLE);
}
/* return 1 when enable 0 when disable
 */
inline bool GetRepeate() {
  return (bool)SendMessage(plugin.hwndParent, WM_WA_IPC, 0, IPC_GET_REPEAT);
}

/*
** If 'status' is 1 then shuffle is turned on.
** If 'status' is 0 then shuffle is turned off.
*/
inline bool SetShuffle(bool status) {
  return (bool)SendMessage(plugin.hwndParent, WM_WA_IPC, status,
                           IPC_SET_SHUFFLE);
}
/*
** If 'status' is 1 then shuffle is turned on.
** If 'status' is 0 then shuffle is turned off.
*/
inline bool SetRepeate(bool status) {
  return (bool)SendMessage(plugin.hwndParent, WM_WA_IPC, status,
                           IPC_SET_REPEAT);
}
enum GETWNDTYPE {
  EQ = IPC_GETWND_EQ,
  PE = IPC_GETWND_PE,
  MB = IPC_GETWND_MB,
  VIDEO = IPC_GETWND_VIDEO
};
/*
** returns the HWND of the window specified
** I can't imagine why this is necessary if you are already passing the HWND to
* the function
**  EQ = IPC_GETWND_EQ,
**  PE = IPC_GETWND_PE,
**  MB = IPC_GETWND_MB,
**  VIDEO = IPC_GETWND_VIDEO
*/
inline HWND GetRepeate(GETWNDTYPE type) {
  return (HWND)SendMessage(plugin.hwndParent, WM_WA_IPC, type, IPC_GETWND);
}
/*
** gets the title of the playlist entry [index].
** returns a pointer to it. returns NULL on error.
*/
inline char *GetPlaylistTitle(int value) {
  return (char *)SendMessage(plugin.hwndParent, WM_WA_IPC, value,
                             IPC_GETPLAYLISTTITLE);
}

/*
** gets the title of the playlist entry [index].
** returns a pointer to it. returns NULL on error.
*/
inline wchar_t *GetPlaylistTitleW(int value) {
  return (wchar_t *)SendMessage(plugin.hwndParent, WM_WA_IPC, value,
                                IPC_GETPLAYLISTTITLEW);
}

enum OUTPUTTIME { TrackElapsed = 0, TrackLengthsec = 1, TrackLengthms = 2 };

/*
** if mode TrackElapsed return position in ms of the currently playing track
** if mode TrackLengthsec return track length in seconds
** if mode TrackLengthms return track length in milliseconds
** return -1 are no track (or possibly if Winamp cannot get the length)
*/
inline int GetTrackTime(OUTPUTTIME mode) {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, mode,
                          IPC_GETOUTPUTTIME);
}

/*
** jump to time in milliseconds
*/
inline char *JumpToTime(int time) {
  return (char *)SendMessage(plugin.hwndParent, WM_WA_IPC, time,
                             IPC_JUMPTOTIME);
}

inline int getPan() {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, -666, IPC_SETPANNING);
}
inline int setPan(int pan) {
  return (int)SendMessage(plugin.hwndParent, WM_WA_IPC, pan, IPC_SETPANNING);
}
} // namespace Winamp