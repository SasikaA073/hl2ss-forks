
#include <mferror.h>
#include "custom_media_buffers.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Media.Capture.Frames.h>
#include <winrt/Windows.Graphics.Imaging.h>

using namespace winrt::Windows::Media::Capture::Frames;
using namespace winrt::Windows::Graphics::Imaging;

//-----------------------------------------------------------------------------
// IUnknown Methods
//-----------------------------------------------------------------------------

// OK
ULONG SoftwareBitmapBuffer::AddRef()
{
    return InterlockedIncrement(&m_nRefCount);
}

// OK
ULONG SoftwareBitmapBuffer::Release()
{
    ULONG uCount = InterlockedDecrement(&m_nRefCount);
    if (uCount == 0) { delete this; }
    return uCount;
}

// OK
HRESULT SoftwareBitmapBuffer::QueryInterface(REFIID iid, void** ppv)
{
    if (!ppv) { return E_POINTER; }

    *ppv = NULL;

    if      (iid == IID_IUnknown)       { *ppv = static_cast<IUnknown*>(this); }
    else if (iid == IID_IMFMediaBuffer) { *ppv = static_cast<IMFMediaBuffer*>(this); }
    else                                { return E_NOINTERFACE; }

    AddRef();
    return S_OK;
}

//-----------------------------------------------------------------------------
// IMFMediaBuffer Methods
//-----------------------------------------------------------------------------

// OK
HRESULT SoftwareBitmapBuffer::GetCurrentLength(DWORD* pcbCurrentLength)
{
    *pcbCurrentLength = m_curLength;
    return S_OK;
}

// OK
HRESULT SoftwareBitmapBuffer::GetMaxLength(DWORD* pcbMaxLength)
{
    *pcbMaxLength = m_maxLength;
    return S_OK;
}

// OK
HRESULT SoftwareBitmapBuffer::Lock(BYTE** ppbBuffer, DWORD* pcbMaxLength, DWORD* pcbCurrentLength)
{
    *ppbBuffer = m_pBase;
    if (pcbMaxLength) { *pcbMaxLength = m_maxLength; }
    if (pcbCurrentLength) { *pcbCurrentLength = m_curLength; }
    return S_OK;
}

// OK
HRESULT SoftwareBitmapBuffer::SetCurrentLength(DWORD cbCurrentLength)
{
    m_curLength = cbCurrentLength;
    return S_OK;
}

// OK
HRESULT SoftwareBitmapBuffer::Unlock()
{
    return S_OK;
}

//-----------------------------------------------------------------------------
// SoftwareBitmapBuffer Methods
//-----------------------------------------------------------------------------

// OK
SoftwareBitmapBuffer::SoftwareBitmapBuffer(MediaFrameReference const &ref) : m_bmp(nullptr), m_buf(nullptr)
{
    UINT32 length;

    m_nRefCount = 1;
    m_bmp       = ref.VideoMediaFrame().SoftwareBitmap();
    m_buf       = m_bmp.LockBuffer(BitmapBufferAccessMode::Read);
    m_ref       = m_buf.CreateReference();
    m_bba       = m_ref.as<Windows::Foundation::IMemoryBufferByteAccess>();

    m_bba->GetBuffer(&m_pBase, &length);

    m_maxLength = length;
    m_curLength = length;
}

// OK
SoftwareBitmapBuffer::~SoftwareBitmapBuffer()
{
                   m_bba = nullptr;
    m_ref.Close(); m_ref = nullptr;
    m_buf.Close(); m_buf = nullptr;
    m_bmp.Close(); m_bmp = nullptr;
}

// OK
HRESULT SoftwareBitmapBuffer::CreateInstance(SoftwareBitmapBuffer** ppBuffer, MediaFrameReference const& ref)
{
    *ppBuffer = new SoftwareBitmapBuffer(ref);
    return S_OK;
}
