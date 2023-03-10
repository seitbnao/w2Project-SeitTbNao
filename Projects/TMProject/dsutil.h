#pragma once

#include <mmsystem.h> 
#include <mmreg.h> 
#include <dsound.h> 

// CPP Files: http://read.pudn.com/downloads154/sourcecode/windows/directx/683385/[Beginning[1].DirectX9]%E4%BB%A3%E7%A0%81/chapter10/example2/dsutil.cpp__.htm

//----------------------------------------------------------------------------- 
// Classes used by this header 
//----------------------------------------------------------------------------- 
class CSoundManager;
class CSound;
class CStreamingSound;
class CWaveFile;

constexpr auto MAX_SOUNDLIST = 512;

#if !defined DDERR_NOTINITIALIZED
#define DDERR_NOTINITIALIZED 0x800401F0
#endif

//----------------------------------------------------------------------------- 
// Typing macros  
//----------------------------------------------------------------------------- 
#define WAVEFILE_READ   1 
#define WAVEFILE_WRITE  2 

#define DSUtil_StopSound(s)         { if(s) s->Stop(); } 
#define DSUtil_PlaySound(s)         { if(s) s->Play( 0, 0 ); } 
#define DSUtil_PlaySoundLooping(s)  { if(s) s->Play( 0, DSBPLAY_LOOPING ); } 

struct stSoundData
{
	CSound* pSoundData;
	char szFileName[256];
	int nChannel;
};

//----------------------------------------------------------------------------- 
// ItemName: class CSoundManager 
// Desc:  
//----------------------------------------------------------------------------- 
class CSoundManager
{
protected:
	LPDIRECTSOUND8 m_pDS;
	stSoundData m_stSoundDataList[MAX_SOUNDLIST];

public:
	int m_nSoundVolume;
	int m_bMute;	
	LPDIRECTSOUND3DLISTENER m_pDSListener;
	DS3DLISTENER m_dsListenerParams;

public:
	CSoundManager();
	~CSoundManager();

	int LoadSoundData();
	HRESULT Initialize(HWND hWnd, DWORD dwCoopLevel, DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);
	inline  LPDIRECTSOUND8 GetDirectSound() { return m_pDS; }
	HRESULT SetPrimaryBufferFormat(DWORD dwPrimaryChannels, DWORD dwPrimaryFreq, DWORD dwPrimaryBitRate);
	HRESULT Get3DListenerInterface(LPDIRECTSOUND3DLISTENER* ppDSListener);

	HRESULT Create(CSound** ppSound, LPTSTR strWaveFileName, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1);
	HRESULT CreateFromMemory(CSound** ppSound, BYTE* pbData, ULONG ulDataSize, LPWAVEFORMATEX pwfx, DWORD dwCreationFlags = 0, GUID guid3DAlgorithm = GUID_NULL, DWORD dwNumBuffers = 1);
	HRESULT CreateStreaming(CStreamingSound** ppStreamingSound, LPTSTR strWaveFileName, DWORD dwCreationFlags, GUID guid3DAlgorithm, DWORD dwNotifyCount, DWORD dwNotifySize, HANDLE hNotifyEvent);
	CSound* GetSoundData(int nIndex);
	void SetSoundVolumeByIndex(int nIndex, int nVolume);
	void SetSoundVolume(int nVolume);
};

//----------------------------------------------------------------------------- 
// ItemName: class CSound 
// Desc: Encapsulates functionality of a DirectSound buffer. 
//----------------------------------------------------------------------------- 
class CSound
{
protected:
	LPDIRECTSOUNDBUFFER* m_apDSBuffer;
	DWORD                m_dwDSBufferSize;
	CWaveFile* m_pWaveFile;
	DWORD                m_dwNumBuffers;
	DWORD                m_dwCreationFlags;

	HRESULT RestoreBuffer(LPDIRECTSOUNDBUFFER pDSB, BOOL* pbWasRestored);

public:
	CSound(LPDIRECTSOUNDBUFFER* apDSBuffer, DWORD dwDSBufferSize, DWORD dwNumBuffers, CWaveFile* pWaveFile);
	virtual ~CSound();

	unsigned int GetBufferCount();
	HRESULT Get3DBufferInterface(DWORD dwIndex, LPDIRECTSOUND3DBUFFER* ppDS3DBuffer);
	HRESULT FillBufferWithSound(LPDIRECTSOUNDBUFFER pDSB, BOOL bRepeatWavIfBufferLarger);
	LPDIRECTSOUNDBUFFER GetFreeBuffer();
	LPDIRECTSOUNDBUFFER GetBuffer(DWORD dwIndex);

	HRESULT Play(DWORD dwPriority = 0, DWORD dwFlags = 0, LONG lVolume = 0, LONG lFrequency = -1, LONG lPan = 0);
	HRESULT Play3D(LPDS3DBUFFER p3DBuffer, DWORD dwPriority = 0, DWORD dwFlags = 0, LONG lFrequency = 0);
	HRESULT Stop();
	HRESULT Reset();
	BOOL    IsSoundPlaying();
};

//----------------------------------------------------------------------------- 
// ItemName: class CStreamingSound 
// Desc: Encapsulates functionality to play a wave file with DirectSound.   
//       The Create() method loads a chunk of wave file into the buffer,  
//       and as sound plays more is written to the buffer by calling  
//       HandleWaveStreamNotification() whenever hNotifyEvent is signaled. 
//----------------------------------------------------------------------------- 
class CStreamingSound : public CSound
{
protected:
	DWORD m_dwLastPlayPos;
	DWORD m_dwPlayProgress;
	DWORD m_dwNotifySize;
	DWORD m_dwNextWriteOffset;
	BOOL  m_bFillNextNotificationWithSilence;

public:
	CStreamingSound(LPDIRECTSOUNDBUFFER pDSBuffer, DWORD dwDSBufferSize, CWaveFile* pWaveFile, DWORD dwNotifySize);
	~CStreamingSound();

	HRESULT HandleWaveStreamNotification(BOOL bLoopedPlay);
	HRESULT Reset();
};

//----------------------------------------------------------------------------- 
// ItemName: class CWaveFile 
// Desc: Encapsulates reading or writing sound data to or from a wave file 
//----------------------------------------------------------------------------- 
class CWaveFile
{
public:
	WAVEFORMATEX* m_pwfx;        // Pointer to WAVEFORMATEX structure 
	HMMIO         m_hmmio;       // MM I/O handle for the WAVE 
	MMCKINFO      m_ck;          // Multimedia RIFF chunk 
	MMCKINFO      m_ckRiff;      // Use in opening a WAVE file 
	DWORD         m_dwSize;      // The size of the wave file 
	MMIOINFO      m_mmioinfoOut;
	DWORD         m_dwFlags;
	BOOL          m_bIsReadingFromMemory;
	BYTE* m_pbData;
	BYTE* m_pbDataCur;
	ULONG         m_ulDataSize;
	CHAR* m_pResourceBuffer;

protected:
	HRESULT ReadMMIO();
	HRESULT WriteMMIO(WAVEFORMATEX* pwfxDest);

public:
	CWaveFile();
	~CWaveFile();

	HRESULT Open(LPTSTR strFileName, WAVEFORMATEX* pwfx, DWORD dwFlags);
	HRESULT OpenFromMemory(BYTE* pbData, ULONG ulDataSize, WAVEFORMATEX* pwfx, DWORD dwFlags);
	HRESULT Close();

	HRESULT Read(BYTE* pBuffer, DWORD dwSizeToRead, DWORD* pdwSizeRead);
	HRESULT Write(UINT nSizeToWrite, BYTE* pbData, UINT* pnSizeWrote);

	DWORD   GetSize();
	HRESULT ResetFile();
	WAVEFORMATEX* GetFormat() { return m_pwfx; };
};
