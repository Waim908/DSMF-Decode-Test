# DirectShow 与 Media Foundation 支持格式统计

> 基于项目中 `DirectShow/`、`media-foundation/`、`medfound/` 文档目录及源代码分析汇总。

---

## 1. 视频解码编码格式

| 编码格式 | DirectShow | Media Foundation | 硬件加速 (DXVA2) | 硬件加速 (D3D11) | 硬件加速 (D3D12) |
|:---------|:----------:|:----------------:|:----------------:|:----------------:|:----------------:|
| H.264 / AVC | ✅ (VCM/Native) | ✅ (内置 MFT) | ✅ | ✅ | ✅ |
| H.265 / HEVC | ✅ (Native) | ✅ (内置 MFT) | ✅ | ✅ | ❌ |
| MPEG-1 | ✅ (Native) | ✅ | ✅ (VLD) | ❌ | ❌ |
| MPEG-2 | ✅ (Native) | ✅ (内置 MFT) | ✅ (VLD) | ❌ | ❌ |
| MPEG-4 Part 2 | ✅ (VCM) | ✅ | ❌ | ❌ | ❌ |
| VC-1 (WMV9 Advanced) | ✅ (Native) | ✅ | ✅ | ❌ | ❌ |
| WMV3 (WMV9) | ✅ (Native) | ✅ | ❌ | ❌ | ❌ |
| VP8 | ❌ | ✅ | ❌ | ❌ | ❌ |
| VP9 | ❌ | ✅ | ✅ (DXVA Spec) | ❌ | ❌ |
| AV1 | ❌ | ✅ (Win10+) | ❌ | ❌ | ❌ |
| MJPEG | ✅ (VCM) | ✅ | ❌ | ❌ | ❌ |
| DV (DVSD/DVCS) | ✅ (VCM) | ✅ | ❌ | ❌ | ❌ |
| H.261 | ✅ (DXVA Spec) | ❌ | ✅ (DXVA1) | ❌ | ❌ |
| H.263 | ✅ (DXVA Spec) | ❌ | ✅ (DXVA1) | ❌ | ❌ |

---

## 2. 音频解码编码格式

| 编码格式 | DirectShow | Media Foundation | 备注 |
|:---------|:----------:|:----------------:|:-----|
| PCM (WAV) | ✅ (ACM) | ✅ (内置 MFT) | 8/16/24/32-bit |
| IEEE Float PCM | ✅ | ✅ | 32/64-bit 浮点 |
| AAC (LC/HE) | ✅ | ✅ (内置 MFT) | ADTS/RAW/LATM |
| MP3 (MPEG-1 Layer 3) | ✅ (ACM) | ✅ (内置 MFT) | |
| WMA8 / WMA9 | ✅ (ACM) | ✅ (内置 MFT) | |
| WMA9 Professional | ✅ | ✅ | |
| WMA9 Lossless | ✅ | ✅ | |
| AC-3 (Dolby Digital) | ✅ | ✅ | |
| E-AC3 (Dolby Digital Plus) | ✅ | ✅ | |
| DTS | ✅ | ✅ | |
| Vorbis | ❌ | ✅ | |
| FLAC | ❌ | ✅ (Win10+) | |
| Opus | ❌ | ✅ (Win10+) | |
| ALAC (Apple Lossless) | ❌ | ✅ (Win10+) | |
| AMR-NB / AMR-WB | ❌ | ✅ (Win8.1+) | |
| MPEG-1 Audio | ✅ | ✅ | |
| MPEG-2 Audio | ✅ | ✅ | |

---

## 3. 容器格式 (文件格式)

| 容器格式 | DirectShow | Media Foundation | 支持的典型编码 |
|:---------|:----------:|:----------------:|:---------------|
| AVI (.avi) | ✅ (AVI Splitter) | ✅ | 任意 VCM/ACM 编码 |
| MP4 (.mp4) | ❌ | ✅ (MPEG-4 File Source) | H.264, H.265, AAC, MPEG-4 |
| MKV (.mkv) | ❌ | ✅ (Win10+) | H.264, H.265, VP8, VP9, AV1, AAC, FLAC, Opus |
| WMV / ASF (.wmv/.asf) | ✅ (WM ASF Reader) | ✅ | WMV3, VC-1, WMA |
| MOV (.mov) | ❌ | ✅ | H.264, H.265, AAC |
| FLV (.flv) | ❌ | ✅ | H.264, AAC, MP3 |
| WebM (.webm) | ❌ | ✅ | VP8, VP9, Vorbis, Opus |
| M4V (.m4v) | ❌ | ✅ | H.264, AAC |
| WAV (.wav) | ✅ (WAV Source) | ✅ | PCM, Float PCM |
| MP3 (.mp3) | ✅ | ✅ | MP3 |
| AAC (.aac) | ❌ | ✅ | AAC |
| FLAC (.flac) | ❌ | ✅ (Win10+) | FLAC |
| OGG (.ogg) | ❌ | ✅ | Vorbis, Opus |
| WMA (.wma) | ✅ (WM ASF Reader) | ✅ | WMA |
| M4A (.m4a) | ❌ | ✅ | AAC, ALAC |
| DVD (VOB) | ✅ (DVD Navigator) | ❌ | MPEG-2, AC-3, DTS, LPCM |
| MPEG-PS/TS (.mpg/.ts) | ✅ | ✅ | MPEG-2, H.264, AAC, MP3 |

---

## 4. 硬件加速支持汇总

| 加速方式 | 支持的编解码器 | 系统要求 | 渲染器 |
|:---------|:---------------|:---------|:-------|
| **DXVA2** (DirectShow) | H.264, HEVC, MPEG-2, VC-1 | Vista+ | VMR-9 / EVR |
| **DXVA2** (Media Foundation) | H.264, HEVC | Vista+ | EVR |
| **D3D11** (Media Foundation) | H.264, HEVC | Win8+ | 自定义 Presenter |
| **D3D12** (Media Foundation) | H.264 (仅软件回退) | Win10+ | 自定义 Presenter |

### DXVA2 硬件解码细节
- **优先级**: H.264 > HEVC > 其他可用解码器
- **输出格式**: NV12 (YUV 4:2:0)
- **需要**: D3D9 设备 + DXVA2 解码器服务 + DXVA2 视频处理器

### D3D11 硬件解码细节
- **支持 Profile**: H.264 VLD, HEVC VLD
- **输出格式**: DXGI_FORMAT_NV12
- **需要**: D3D11 设备 + ID3D11VideoDevice

### D3D12 硬件解码细节
- **当前状态**: 仅支持 H.264 Profile 检测，MinGW 构建下回退到软件解码
- **支持 Profile**: D3D12_VIDEO_DECODE_PROFILE_H264
- **限制**: MinGW 编译器对 D3D12 视频 API 支持不完整

---

## 5. 数据来源

- `DirectShow/audio-subtypes.md` - 音频子类型 GUID 列表
- `DirectShow/avi-draw-filter.md` - 视频子类型 (MJPEG, DV 等)
- `DirectShow/about-directx-video-acceleration.md` - DXVA 支持的编解码器
- `medfound/audio-subtype-guids.md` - Media Foundation 音频格式完整列表
- `medfound/h-264-video-decoder.md` - H.264 解码器规格
- `medfound/h-265---hevc-video-decoder.md` - HEVC 解码器规格
- `medfound/about-dxva-2-0.md` - DXVA 2.0 规格说明
- `src/mf_decoder.c` - 项目中实际识别的编解码器 (mf_subtype_to_name)
- `src/dxva2_helper.c` - DXVA2 硬件解码实现
- `src/d3d11_video_helper.c` - D3D11 硬件解码实现
- `src/d3d12_video_helper.c` - D3D12 硬件解码实现

---

# 以下数据均来自原版 **wine11.11**


## 6. Wine 中的音频编解码器实现（绕过 GStreamer）

Wine 通过 ACM（音频编解码器管理器）和 MFT（媒体基础转换）模块实现音频编解码，这些模块可以绕过 GStreamer 框架直接处理音频数据。

### 6.1 DirectShow ACM 模块

Wine 包含以下 ACM 音频编解码器模块：

#### msgsm32.acm - GSM 6.10 编解码器
- **文件位置**: `wine/dlls/msgsm32.acm/msgsm32.c`
- **支持格式**: 
  - 输入/输出: WAVE_FORMAT_PCM (PCM) ↔ WAVE_FORMAT_GSM610 (GSM 6.10)
- **技术规格**:
  - 压缩比: 约 10:1 (640 字节 PCM → 65 字节 GSM)
  - 采样率: 8000, 11025, 22050, 44100, 48000, 96000 Hz
  - 通道数: 单声道 (1 通道)
  - PCM 位深: 16 位，块对齐 2 字节
  - GSM 块对齐: 65 字节，每块 320 个样本
  - 使用 libgsm 库，支持 GSM_OPT_WAV49 选项

#### imaadp32.acm - IMA ADPCM 编解码器
- **文件位置**: `wine/dlls/imaadp32.acm/imaadp32.c`
- **支持格式**:
  - 输入/输出: WAVE_FORMAT_PCM (PCM) ↔ WAVE_FORMAT_IMA_ADPCM (IMA ADPCM)
- **技术规格**:
  - 压缩比: 约 4:1 (16 位 PCM → 4 位 ADPCM)
  - 采样率: 8000, 11025, 22050, 44100 Hz
  - 通道数: 单声道和立体声
  - PCM 位深: 8/16 位
  - ADPCM 位深: 4 位

#### msg711.acm - G.711 编解码器
- **文件位置**: `wine/dlls/msg711.acm/msg711.c`
- **支持格式**:
  - 输入/输出: WAVE_FORMAT_PCM (PCM) ↔ WAVE_FORMAT_ALAW (A-Law) / WAVE_FORMAT_MULAW (MU-Law)
- **技术规格**:
  - 压缩比: 2:1 (16 位 PCM → 8 位 G.711)
  - 采样率: 8000, 11025, 22050, 44100 Hz
  - 通道数: 单声道和立体声
  - PCM 位深: 16 位
  - G.711 位深: 8 位

#### msadp32.acm - Microsoft ADPCM 编解码器
- **文件位置**: `wine/dlls/msadp32.acm/msadp32.c`
- **支持格式**:
  - 输入/输出: WAVE_FORMAT_PCM (PCM) ↔ WAVE_FORMAT_ADPCM (MS ADPCM)
- **技术规格**:
  - 压缩比: 约 4:1 (16 位 PCM → 4 位 ADPCM)
  - 采样率: 8000, 11025, 22050, 44100 Hz
  - 通道数: 单声道和立体声
  - PCM 位深: 8/16 位
  - ADPCM 位深: 4 位

#### l3codeca.acm - MPEG Layer 3 (MP3) 编解码器
- **文件位置**: `wine/dlls/l3codeca.acm/mpegl3.c`
- **支持格式**:
  - 输入/输出: WAVE_FORMAT_PCM (PCM) ↔ WAVE_FORMAT_MPEG / WAVE_FORMAT_MPEGLAYER3 (MP3)
- **技术规格**:
  - 采样率: 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 Hz
  - 通道数: 单声道和立体声
  - PCM 位深: 8/16 位
  - 使用 mpg123 库进行解码

### 6.2 Media Foundation MFT 模块

Wine 包含以下 MFT 媒体基础转换模块：

#### mfh264enc - H.264 编码器
- **文件位置**: `wine/dlls/mfh264enc/mfh264enc.c`
- **支持格式**:
  - 输入: IYUV, YV12, NV12, YUY2 (原始视频格式)
  - 输出: H.264 (编码视频)
- **技术规格**:
  - 使用 CLSID_wg_h264_encoder 进行编码
  - 支持多种原始视频格式转换为 H.264

#### msauddecmft - AAC 音频解码器
- **文件位置**: `wine/dlls/msauddecmft/msauddecmft.c`
- **支持格式**:
  - 输入: AAC, RAW AAC, ADTS
  - 输出: 浮点 PCM (MFAudioFormat_Float)
- **技术规格**:
  - 使用 CLSID_wg_aac_decoder 进行解码
  - 支持多种 AAC 格式变体

### 6.3 容器格式支持

#### mfmp4srcsnk - MP4 源和接收器
- **文件位置**: `wine/dlls/mfmp4srcsnk/mfmp4srcsnk.c`
- **功能**: 处理 MPEG-4 字节流，支持 MP4 容器格式
- **支持**: MPEG4 字节流处理，MP3 和 MPEG-4 接收器

#### mfsrcsnk - 通用源和接收器
- **文件位置**: `wine/dlls/mfsrcsnk/media_source.c`
- **功能**: 提供通用的媒体源和接收器框架
- **支持**: 基础的媒体源和接收器架构

#### mfasfsrcsnk - ASF 源和接收器
- **文件位置**: `wine/dlls/mfasfsrcsnk/mfasfsrcsnk.c`
- **功能**: 处理 ASF 字节流，支持 WMV/WMA 容器格式
- **支持**: ASF 字节流处理

### 6.4 Wine 音频编解码器特性

#### 共同特性
- 所有 ACM 模块都实现了标准的 ACM 驱动程序接口
- 支持格式验证、格式建议、流大小计算和流转换
- 提供格式标签详细信息和格式详细信息查询
- 支持流打开、关闭、准备和取消准备操作

#### 依赖关系
- **libgsm**: GSM 6.10 编解码器依赖
- **mpg123**: MP3 编解码器依赖
- **winegstreamer**: GStreamer 集成（可被绕过）

#### 注册机制
- ACM 模块通过 DLL 导出 DriverProc 函数
- MFT 模块通过 DllGetClassObject 和 DllRegisterServer 注册
- 支持标准的 COM 类工厂模式

### 6.5 使用场景

这些编解码器模块在以下情况下特别有用：
1. **绕过 GStreamer**: 当需要直接使用 Windows 原生编解码器时
2. **性能优化**: 某些编解码器可能比 GStreamer 实现更高效
3. **格式兼容性**: 确保与特定 Windows 应用程序的兼容性
4. **调试和测试**: 验证编解码器行为和性能

---

## 7. 数据来源更新

### Wine 源代码分析
- `wine/dlls/msgsm32.acm/msgsm32.c` - GSM 6.10 编解码器实现
- `wine/dlls/imaadp32.acm/imaadp32.c` - IMA ADPCM 编解码器实现
- `wine/dlls/msg711.acm/msg711.c` - G.711 编解码器实现
- `wine/dlls/msadp32.acm/msadp32.c` - Microsoft ADPCM 编解码器实现
- `wine/dlls/l3codeca.acm/mpegl3.c` - MP3 编解码器实现
- `wine/dlls/mfh264enc/mfh264enc.c` - H.264 编码器实现
- `wine/dlls/msauddecmft/msauddecmft.c` - AAC 解码器实现
- `wine/dlls/mfmp4srcsnk/mfmp4srcsnk.c` - MP4 容器支持
- `wine/dlls/mfsrcsnk/media_source.c` - 通用媒体源实现
- `wine/dlls/mfasfsrcsnk/mfasfsrcsnk.c` - ASF 容器支持

## 8. Wine 中的视频编解码器实现（绕过 GStreamer）

Wine 通过 DirectShow DMO（DirectX Media Object）和 Media Foundation MFT（Media Foundation Transform）模块实现视频编解码，这些模块可以绕过 GStreamer 框架直接处理视频数据。

### 8.1 DirectShow DMO 模块

#### wmvdecod.dll - WMV 视频解码器
- **文件位置**: `wine/dlls/wmvdecod/wmvdecod.c`
- **支持格式**:
  - **输入格式**:
    - WMV1 (Windows Media Video 7)
    - WMV2 (Windows Media Video 8)
    - WMV3 (Windows Media Video 9)
    - WMVA (WMV 9 Advanced Profile)
    - WVC1 (VC-1)
    - WMVP (WMV 9.1 Image)
    - WVP2 (WMV 9.1 Image v2)
    - VC1S (VC-1 Simple Profile)
  - **输出格式**:
    - YUV 格式: YV12, YUY2, UYVY, YVYU, NV11, NV12
    - RGB 格式: RGB32, RGB24, RGB565, RGB555, RGB8
- **技术规格**:
  - 作为 DirectShow DMO 和 Media Foundation MFT 双重注册
  - 支持同步处理模式 (MFT_ENUM_FLAG_SYNCMFT)
  - 使用 GStreamer 作为后端解码引擎
  - 支持硬件加速通过 DXVA2 和 D3D11

#### msmpeg2vdec.dll - H.264 视频解码器
- **文件位置**: `wine/dlls/msmpeg2vdec/msmpeg2vdec.c`
- **支持格式**:
  - **输入格式**:
    - H.264 (MPEG-4 AVC)
    - H.264 ES (Elementary Stream)
  - **输出格式**:
    - NV12 (YUV 4:2:0 Semi-Planar)
    - YV12 (YUV 4:2:0 Planar)
    - IYUV (YUV 4:2:0 Planar)
    - I420 (YUV 4:2:0 Planar)
    - YUY2 (YUV 4:2:2 Packed)
- **技术规格**:
  - 作为 Media Foundation MFT 注册
  - 支持 H.264 视频加速 (CODECAPI_AVDecVideoAcceleration_H264)
  - 使用 GStreamer 作为后端解码引擎

### 8.2 Media Foundation MFT 模块

#### winegstreamer 视频解码器
- **文件位置**: `wine/dlls/winegstreamer/video_decoder.c`
- **支持格式**:
  - **H.264 解码器**:
    - 输入: MFVideoFormat_H264, MFVideoFormat_H264_ES
    - 输出: NV12, YV12, IYUV, I420, YUY2
  - **WMV 解码器**:
    - 输入: WMV1, WMV2, WMVA, WMVP, WVP2, WMV_Unknown, WVC1, WMV3, VC1S
    - 输出: NV12, YV12, IYUV, I420, YUY2, UYVY, YVYU, NV11, RGB32, RGB24, RGB565, RGB555, RGB8
  - **Indeo Video 5 (IV50) 解码器**:
    - 输入: MFVideoFormat_IV50
    - 输出: YV12, YUY2, NV11, NV12, RGB32, RGB24, RGB565, RGB555, RGB8
- **技术规格**:
  - 使用 GStreamer 作为后端解码引擎
  - 支持 D3D9 和 D3D11 硬件加速
  - 支持格式协商和输出类型选择
  - 支持视频处理和格式转换

### 8.3 音频解码器

#### wmadmod.dll - WMA 音频解码器
- **文件位置**: `wine/dlls/wmadmod/wmadmod.c`
- **支持格式**:
  - **输入格式**:
    - MSAUDIO1 (WMA 1)
    - WMAudioV8 (WMA 8)
    - WMAudioV9 (WMA 9)
    - WMAudio_Lossless (WMA 9 Lossless)
  - **输出格式**:
    - PCM
    - IEEE Float
- **技术规格**:
  - 作为 DirectShow DMO 和 Media Foundation MFT 双重注册
  - 支持多种 WMA 格式变体

#### mp3dmod.dll - MP3 音频解码器
- **文件位置**: `wine/dlls/mp3dmod/mp3dmod.c`
- **支持格式**:
  - **输入格式**:
    - MP3 (MPEG-1 Layer 3)
  - **输出格式**:
    - PCM
- **技术规格**:
  - 作为 DirectShow DMO 和 Media Foundation MFT 双重注册
  - 使用 mpg123 库进行解码

#### msauddecmft.dll - AAC 音频解码器
- **文件位置**: `wine/dlls/msauddecmft/msauddecmft.c`
- **支持格式**:
  - **输入格式**:
    - AAC
    - RAW AAC
    - ADTS (Audio Data Transport Stream)
  - **输出格式**:
    - Float (IEEE 浮点)
    - PCM
- **技术规格**:
  - 作为 Media Foundation MFT 注册
  - 支持多种 AAC 格式变体

### 8.4 视频解码器特性

#### 共同特性
- 所有解码器都支持同步处理模式
- 支持格式协商和输出类型选择
- 提供 DMO 和 MFT 双重接口
- 使用 GStreamer 作为后端解码引擎

#### 硬件加速支持
- **DXVA2**: 通过 D3D9 设备支持硬件加速
- **D3D11**: 通过 D3D11 设备支持硬件加速
- **D3D12**: 部分支持，MinGW 构建下回退到软件解码

#### 输出格式优化
- 优先使用 NV12 格式以获得最佳性能
- 支持多种 YUV 和 RGB 格式以适应不同渲染需求
- 支持格式转换和缩放

### 8.5 使用场景

这些视频解码器模块在以下情况下特别有用：
1. **绕过 GStreamer**: 当需要直接使用 Windows 原生解码器时
2. **性能优化**: 某些解码器可能比 GStreamer 实现更高效
3. **格式兼容性**: 确保与特定 Windows 应用程序的兼容性
4. **硬件加速**: 利用 GPU 加速解码过程
5. **调试和测试**: 验证解码器行为和性能

### 8.6 技术实现细节

#### WMV 解码器架构
- **DirectShow 路径**: wmvdecod.dll → GStreamer → 原始视频
- **Media Foundation 路径**: winegstreamer.dll → GStreamer → 原始视频
- **双重注册**: 同时支持 DirectShow DMO 和 Media Foundation MFT 接口

#### H.264 解码器架构
- **Media Foundation 路径**: msmpeg2vdec.dll 或 winegstreamer.dll → GStreamer → 原始视频
- **硬件加速**: 支持 DXVA2 和 D3D11 硬件加速
- **格式协商**: 支持输入输出格式协商和优化

#### 性能考虑
- **格式选择**: 优先选择 NV12 格式以减少内存带宽
- **硬件加速**: 利用 GPU 解码以释放 CPU 资源
- **缓冲区管理**: 支持样本分配和缓冲区管理优化

---

## 9. Wine 中直接实现的解码器（绕过 GStreamer）

> 以下解码器在 Wine 中直接实现，不依赖 GStreamer 框架，可以独立完成音视频解码和播放。

### 9.1 直接实现的视频解码器（VFW 接口）

| 解码器名称 | FourCC 代码 | 支持位深度 | 输出格式 | 实现文件 |
|:-----------|:------------|:----------|:---------|:---------|
| **Cinepak** | `cvid` | 15/16/24/32 位 | RGB555, RGB565, RGB24, RGB32 | `wine/dlls/iccvid/iccvid.c` |
| **Microsoft RLE** | `RLE `, `RLE4`, `RLE8`, `MRLE` | 1/4/8/15/16/24/32 位 | 对应位深度的 DIB | `wine/dlls/msrle32/msrle32.c` |
| **Microsoft Video 1** | `CRAM`, `MSVC`, `WHAM` | 8/16 位 | 8位调色板, RGB555, RGB24 | `wine/dlls/msvidc32/msvideo1.c` |

#### Cinepak 解码器详情
- **算法**: 基于向量量化（Vector Quantization）的自适应向量密度编码
- **块大小**: 4x4 像素块
- **码本**: 每个条带维护 V1（1向量）和 V4（4向量）两个码本
- **颜色空间**: YUV 到 RGB 转换，使用查找表优化
- **压缩功能**: 仅支持解码，不支持编码

#### Microsoft RLE 解码器详情
- **算法**: 游程编码（Run-Length Encoding）
- **支持模式**: RLE4（4位）、RLE8（8位）
- **特性**: 支持时间压缩（帧间差分）、质量设置、压缩到目标大小
- **功能**: 支持编码和解码双向转换

#### Microsoft Video 1 解码器详情
- **算法**: 块编码（Block-based Coding），4x4 像素块
- **编码模式**: 1色、2色、8色块编码
- **调色板**: 支持 256 色调色板模式
- **颜色转换**: 支持 RGB555 到 RGB24 的转换

### 9.2 直接实现的音频解码器（ACM 接口）

| 解码器名称 | 格式标签 | 压缩比 | 采样率支持 | 通道数 | 实现文件 |
|:-----------|:---------|:-------|:-----------|:-------|:---------|
| **MP3** | `WAVE_FORMAT_MPEGLAYER3` (0x0055) | 可变 | 8000-48000 Hz | 单声道/立体声 | `wine/dlls/l3codeca.acm/mpegl3.c` |
| **IMA ADPCM** | `WAVE_FORMAT_IMA_ADPCM` (0x0011) | 4:1 | 8000-44100 Hz | 单声道/立体声 | `wine/dlls/imaadp32.acm/imaadp32.c` |
| **Microsoft ADPCM** | `WAVE_FORMAT_ADPCM` (0x0002) | 4:1 | 8000-44100 Hz | 单声道/立体声 | `wine/dlls/msadp32.acm/msadp32.c` |
| **G.711 A-Law** | `WAVE_FORMAT_ALAW` (0x0006) | 2:1 | 8000-44100 Hz | 单声道/立体声 | `wine/dlls/msg711.acm/msg711.c` |
| **G.711 MU-Law** | `WAVE_FORMAT_MULAW` (0x0007) | 2:1 | 8000-44100 Hz | 单声道/立体声 | `wine/dlls/msg711.acm/msg711.c` |
| **GSM 6.10** | `WAVE_FORMAT_GSM610` (0x0031) | 10:1 | 8000-96000 Hz | 单声道 | `wine/dlls/msgsm32.acm/msgsm32.c` |

#### MP3 解码器详情
- **依赖库**: mpg123（外部 MP3 解码库）
- **输出格式**: 16 位 PCM，保持相同采样率和通道数
- **支持格式**: WAVE_FORMAT_PCM、WAVE_FORMAT_MPEGLAYER3、WAVE_FORMAT_MPEG
- **采样率**: 8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000 Hz

#### IMA ADPCM 解码器详情
- **算法**: 自适应差分脉冲编码调制（IMA/DVI ADPCM）
- **量化位数**: 4 位
- **块结构**: 每块包含头信息和压缩数据
- **编码支持**: 支持 PCM 到 IMA ADPCM 的编码转换

#### Microsoft ADPCM 解码器详情
- **算法**: Microsoft ADPCM 压缩算法
- **量化位数**: 4 位
- **预测系数**: 使用 7 组系数集
- **增量表**: 使用 16 个增量值

#### G.711 解码器详情
- **标准**: ITU-T G.711 标准
- **变体**: A-Law（欧洲）和 MU-Law（北美）
- **实现**: 使用预计算查找表优化性能
- **转换**: 支持 PCM ↔ A-Law/MU-Law 双向转换

#### GSM 6.10 解码器详情
- **依赖库**: libgsm（外部 GSM 解码库）
- **压缩比**: 约 10:1（640 字节 PCM → 65 字节 GSM）
- **每块样本**: 320 个样本
- **特殊配置**: 需要 GSM_OPT_WAV49 选项支持

### 9.3 通过 winedmo/FFmpeg 实现的解码器（绕过 GStreamer）

Wine 通过 `winedmo` 库（基于 FFmpeg）实现媒体解复用和解码，可以绕过 GStreamer 框架。

#### 支持的容器格式

| 容器格式 | MIME 类型 | 文件扩展名 | 实现文件 |
|:---------|:----------|:-----------|:---------|
| **MP4** | `video/mp4` | `.mp4`, `.m4a`, `.m4v`, `.mov`, `.3gp`, `.3g2` | `wine/dlls/mfmp4srcsnk/mfmp4srcsnk.c` |
| **AVI** | `video/avi` | `.avi` | `wine/dlls/mfsrcsnk/media_source.c` |
| **WAV** | `audio/wav` | `.wav` | `wine/dlls/mfsrcsnk/media_source.c` |
| **MP3** | `audio/mp3` | `.mp3` | `wine/dlls/mfsrcsnk/media_source.c` |
| **ASF** | `video/x-ms-asf` | `.asf`, `.wmv`, `.wma` | `wine/dlls/mfasfsrcsnk/mfasfsrcsnk.c` |
| **MPEG** | `video/mpeg` | `.mpg`, `.mpeg` | `wine/dlls/mfsrcsnk/media_source.c` |

#### 支持的音频编码格式（通过 winedmo/FFmpeg）

| 编码格式 | WAVE 格式标签 | 说明 |
|:---------|:--------------|:-----|
| **AAC** | `WAVE_FORMAT_MPEG_HEAAC` | 支持 LC/HE 配置，ADTS/RAW/LATM 封装 |
| **MP1** | `MPEG1WAVEFORMAT` | MPEG Audio Layer 1 |
| **MP3** | `MPEGLAYER3WAVEFORMAT` | MPEG Audio Layer 3 |
| **WMA v1** | `MSAUDIO1WAVEFORMAT` | Windows Media Audio 7 |
| **WMA v2** | `WMAUDIO2WAVEFORMAT` | Windows Media Audio 8 |
| **WMA Pro** | `WMAUDIO3WAVEFORMAT` | Windows Media Audio 9 Professional |
| **WMA Voice** | `WMAUDIO3WAVEFORMAT` | Windows Media Audio Voice |
| **WMA Lossless** | `WMAUDIO3WAVEFORMAT` | Windows Media Audio Lossless |
| **PCM** | `WAVE_FORMAT_PCM` | 无压缩音频 |
| **IEEE Float** | `WAVE_FORMAT_IEEE_FLOAT` | 浮点音频 |

#### 支持的视频编码格式（通过 winedmo/FFmpeg）

| 编码格式 | MF 视频格式 | 说明 |
|:---------|:------------|:-----|
| **H.264/AVC** | `MFVideoFormat_H264` | 支持 byte-stream 格式，带 h264_mp4toannexb 位流过滤器 |
| **MPEG-1 Video** | `MEDIASUBTYPE_MPEG1Payload` | 早期数字视频 |
| **VP9** | `MFVideoFormat_VP90` | Google 开源视频编码 |
| **原始视频** | 多种像素格式 | 无压缩视频 |

#### 支持的像素格式（原始视频）

| FFmpeg 像素格式 | MF 视频格式 | 说明 |
|:----------------|:------------|:-----|
| `AV_PIX_FMT_YUV420P` | `MFVideoFormat_I420` | YUV 4:2:0 平面格式 |
| `AV_PIX_FMT_YUYV422` | `MFVideoFormat_YUY2` | YUV 4:2:2 打包格式 |
| `AV_PIX_FMT_UYVY422` | `MFVideoFormat_UYVY` | YUV 4:2:2 打包格式 |
| `AV_PIX_FMT_BGR0` | `MFVideoFormat_RGB32` | BGRX 32位 |
| `AV_PIX_FMT_RGBA` | `MFVideoFormat_ABGR32` | RGBA 32位 |
| `AV_PIX_FMT_BGRA` | `MFVideoFormat_ARGB32` | BGRA 32位 |
| `AV_PIX_FMT_BGR24` | `MFVideoFormat_RGB24` | BGR 24位 |
| `AV_PIX_FMT_NV12` | `MFVideoFormat_NV12` | YUV 4:2:0 半平面格式 |
| `AV_PIX_FMT_NV21` | `MFVideoFormat_NV21` | YUV 4:2:0 半平面格式 |
| `AV_PIX_FMT_P010` | `MFVideoFormat_P010` | 10-bit YUV 4:2:0 |
| `AV_PIX_FMT_RGB565` | `MFVideoFormat_RGB565` | RGB 16位 5:6:5 |
| `AV_PIX_FMT_RGB555` | `MFVideoFormat_RGB555` | RGB 16位 5:5:5 |
| `AV_PIX_FMT_RGB8` | `MFVideoFormat_RGB8` | 8位索引色 |

### 9.4 解码器依赖关系汇总

| 解码器类型 | 依赖库 | 绕过 GStreamer | 说明 |
|:-----------|:-------|:---------------|:-----|
| Cinepak | 无 | ✅ | 完全独立实现 |
| Microsoft RLE | 无 | ✅ | 完全独立实现 |
| Microsoft Video 1 | 无 | ✅ | 完全独立实现 |
| MP3 | mpg123 | ✅ | 依赖外部 mpg123 库 |
| IMA ADPCM | 无 | ✅ | 完全独立实现 |
| Microsoft ADPCM | 无 | ✅ | 完全独立实现 |
| G.711 | 无 | ✅ | 完全独立实现 |
| GSM 6.10 | libgsm | ✅ | 依赖外部 libgsm 库 |
| AAC (winedmo) | FFmpeg | ✅ | 通过 winedmo 库使用 FFmpeg |
| H.264 (winedmo) | FFmpeg | ✅ | 通过 winedmo 库使用 FFmpeg |
| VP9 (winedmo) | FFmpeg | ✅ | 通过 winedmo 库使用 FFmpeg |
| WMA (winedmo) | FFmpeg | ✅ | 通过 winedmo 库使用 FFmpeg |
| MPEG-1 (winedmo) | FFmpeg | ✅ | 通过 winedmo 库使用 FFmpeg |

### 9.5 GStreamer 绕过配置

Wine 提供了配置选项来控制是否使用 GStreamer：

- **注册表路径**: `HKCU\Software\Wine\MediaFoundation`
- **键名**: `DisableGstByteStreamHandler`
- **值**: DWORD
  - `0` 或不存在：使用 GStreamer（默认）
  - 非 `0`：强制使用原生 winedmo/FFmpeg 实现

### 9.6 数据来源（新增）

- `wine/dlls/iccvid/iccvid.c` - Cinepak 视频解码器实现
- `wine/dlls/msrle32/msrle32.c` - Microsoft RLE 视频解码器实现
- `wine/dlls/msvidc32/msvideo1.c` - Microsoft Video 1 解码器实现
- `wine/dlls/l3codeca.acm/mpegl3.c` - MP3 音频解码器实现
- `wine/dlls/imaadp32.acm/imaadp32.c` - IMA ADPCM 音频解码器实现
- `wine/dlls/msadp32.acm/msadp32.c` - Microsoft ADPCM 音频解码器实现
- `wine/dlls/msg711.acm/msg711.c` - G.711 音频解码器实现
- `wine/dlls/msgsm32.acm/msgsm32.c` - GSM 6.10 音频解码器实现
- `wine/dlls/mfsrcsnk/media_source.c` - Media Foundation 源实现
- `wine/dlls/mfmp4srcsnk/mfmp4srcsnk.c` - MP4 容器支持
- `wine/dlls/mfasfsrcsnk/mfasfsrcsnk.c` - ASF 容器支持
- `wine/dlls/winedmo/unix_demuxer.c` - FFmpeg 解复用器实现
- `wine/dlls/winedmo/unix_media_type.c` - FFmpeg 媒体类型转换

---

## 10. winedmo 模块详解（FFmpeg 集成）

> winedmo 是 Wine 的媒体解复用器模块，通过 FFmpeg 库提供对多种媒体格式的支持，可以绕过 GStreamer 独立工作。

### 10.1 模块架构

```
Windows 应用层
      ↓
winedmo.dll (Windows 侧 - main.c)
      ↓ [Wine Unix 调用机制]
winedmo.so (Unix 侧 - unixlib.c, unix_demuxer.c, unix_media_type.c)
      ↓
FFmpeg 库 (libavformat, libavcodec, libavutil)
```

**核心文件**：
- `wine/dlls/winedmo/main.c` - Windows 侧入口和接口实现
- `wine/dlls/winedmo/unixlib.c` - Unix 侧库初始化和回调管理
- `wine/dlls/winedmo/unix_demuxer.c` - 解复用器核心实现
- `wine/dlls/winedmo/unix_media_type.c` - 媒体类型转换映射

### 10.2 工作流程

1. **初始化阶段**：`DllMain` 调用 `process_attach`，注册流回调函数
2. **解复用器创建**：`winedmo_demuxer_create` 创建 FFmpeg 上下文，设置 IO 上下文
3. **流信息解析**：调用 `avformat_find_stream_info` 获取流信息
4. **读取数据包**：`winedmo_demuxer_read` 通过位流过滤器读取数据包
5. **媒体类型查询**：`winedmo_demuxer_stream_type` 返回流的媒体类型信息
6. **清理销毁**：`winedmo_demuxer_destroy` 释放所有资源

### 10.3 支持的容器格式

| 容器格式 | MIME 类型 | FFmpeg 格式 | 说明 |
|:---------|:----------|:------------|:-----|
| **MP4** | `video/mp4` | `mp4` | MPEG-4 Part 14 容器 |
| **AVI** | `video/avi` | `avi` | Audio Video Interleave |
| **WAV** | `audio/wav` | `wav` | Waveform Audio |
| **ASF** | `video/x-ms-asf` | `asf` | Advanced Systems Format |
| **WMV** | `video/x-ms-wmv` | `asf` | Windows Media Video |
| **WMA** | `audio/x-ms-wma` | `asf` | Windows Media Audio |
| **MPEG** | `video/mpeg` | `mpeg` | MPEG-1/2 系统流 |
| **MP3** | `audio/mp3` | `mp3` | MPEG-1 Audio Layer 3 |

### 10.4 音频编解码器支持

| 编码格式 | WAVE 格式标签 | 格式结构 | 说明 |
|:---------|:--------------|:---------|:-----|
| **AAC** | `WAVE_FORMAT_MPEG_HEAAC` (0x1610) | `HEAACWAVEINFO` | 支持 LC/HE 配置，ADTS/RAW/LATM 封装 |
| **MP1** | `MPEG1WAVEFORMAT` | `MPEG1WAVEFORMAT` | MPEG Audio Layer 1 |
| **MP3** | `MPEGLAYER3WAVEFORMAT` | `MPEGLAYER3WAVEFORMAT` | MPEG Audio Layer 3 |
| **WMA v1** | `MSAUDIO1WAVEFORMAT` | `MSAUDIO1WAVEFORMAT` | Windows Media Audio 7 |
| **WMA v2** | `WMAUDIO2WAVEFORMAT` | `WMAUDIO2WAVEFORMAT` | Windows Media Audio 8 |
| **WMA Pro** | `WMAUDIO3WAVEFORMAT` | `WMAUDIO3WAVEFORMAT` | Windows Media Audio 9 Professional |
| **WMA Voice** | `WMAUDIO3WAVEFORMAT` | `WMAUDIO3WAVEFORMAT` | Windows Media Audio Voice |
| **WMA Lossless** | `WMAUDIO3WAVEFORMAT` | `WMAUDIO3WAVEFORMAT` | Windows Media Audio Lossless |
| **PCM** | `WAVE_FORMAT_PCM` (0x0001) | `WAVEFORMATEX` | 无压缩音频 |
| **IEEE Float** | `WAVE_FORMAT_IEEE_FLOAT` (0x0003) | `WAVEFORMATEX` | 浮点音频 |

### 10.5 视频编解码器支持

| 编码格式 | MF 视频格式 | 说明 |
|:---------|:------------|:-----|
| **H.264/AVC** | `MFVideoFormat_H264` | 支持 byte-stream 格式，带 `h264_mp4toannexb` 位流过滤器 |
| **MPEG-1 Video** | `MEDIASUBTYPE_MPEG1Payload` | 早期数字视频 |
| **VP9** | `MFVideoFormat_VP90` | Google 开源视频编码 |
| **原始视频** | 多种像素格式 | 无压缩视频 |

### 10.6 像素格式映射

| FFmpeg 像素格式 | MF 视频格式 | 说明 |
|:----------------|:------------|:-----|
| `AV_PIX_FMT_YUV420P` | `MFVideoFormat_I420` | YUV 4:2:0 平面格式 |
| `AV_PIX_FMT_YUYV422` | `MFVideoFormat_YUY2` | YUV 4:2:2 打包格式 |
| `AV_PIX_FMT_UYVY422` | `MFVideoFormat_UYVY` | YUV 4:2:2 打包格式 |
| `AV_PIX_FMT_BGR0` | `MFVideoFormat_RGB32` | BGRX 32位 |
| `AV_PIX_FMT_RGBA` | `MFVideoFormat_ABGR32` | RGBA 32位 |
| `AV_PIX_FMT_BGRA` | `MFVideoFormat_ARGB32` | BGRA 32位 |
| `AV_PIX_FMT_BGR24` | `MFVideoFormat_RGB24` | BGR 24位 |
| `AV_PIX_FMT_NV12` | `MFVideoFormat_NV12` | YUV 4:2:0 半平面格式 |
| `AV_PIX_FMT_NV21` | `MFVideoFormat_NV21` | YUV 4:2:0 半平面格式 |
| `AV_PIX_FMT_P010` | `MFVideoFormat_P010` | 10-bit YUV 4:2:0 |
| `AV_PIX_FMT_RGB565` | `MFVideoFormat_RGB565` | RGB 16位 5:6:5 |
| `AV_PIX_FMT_RGB555` | `MFVideoFormat_RGB555` | RGB 16位 5:5:5 |
| `AV_PIX_FMT_RGB8` | `MFVideoFormat_RGB8` | 8位索引色 |

### 10.7 位流过滤器

**H.264 位流过滤器**：
- 过滤器名称：`h264_mp4toannexb`
- 功能：将 MP4 容器中的 AVC 编码格式（带 `avcC` atom）转换为 Annex B 格式（带 start codes）
- 实现位置：`wine/dlls/winedmo/unix_demuxer.c:141-172`

**空位流过滤器**：
- 对于其他流，使用 `av_bsf_get_null_filter` 创建空过滤器
- 空过滤器直接传递数据包，不进行转换

### 10.8 错误处理和回退机制

**错误码体系**：
- `STATUS_SUCCESS` - 操作成功
- `STATUS_NOT_SUPPORTED` - 不支持的格式
- `STATUS_BUFFER_TOO_SMALL` - 缓冲区不足
- `STATUS_END_OF_FILE` - 文件结束
- `STATUS_UNSUCCESSFUL` - 一般错误
- `STATUS_NO_MEMORY` - 内存分配失败

**回退机制**：
1. **MIME 类型回退**：对于未知格式，使用 "video/x-application" 作为回退
2. **流信息查找回退**：仅在必要时调用 `avformat_find_stream_info`
3. **格式标签回退**：当无法确定格式标签时，使用 FFmpeg 的 `av_codec_get_tag` 获取默认标签

---

## 11. winegstreamer 模块详解（GStreamer 集成）

> winegstreamer 是 Wine 中将 GStreamer 多媒体框架桥接到 Windows API 的核心模块，同时支持 DirectShow 和 Media Foundation 两套接口。

### 11.1 模块架构

```
Windows 侧 (winegstreamer.dll)                    Unix 侧 (winegstreamer.so)
+----------------------------+                     +----------------------------+
| main.c (入口点, COM工厂)   |                     | unixlib.c (GStreamer初始化) |
| quartz_parser.c (DS解析)   | ---- WINE_UNIX_CALL | wg_parser.c (解复用)       |
| quartz_transform.c (DS转换)| ==================> | wg_transform.c (转码)      |
| video_decoder.c (MF解码)   |                     | wg_muxer.c (多路复用)      |
| video_encoder.c (MF编码)   |                     | wg_format.c (格式转换)     |
| media_source.c (MF源)      |                     | wg_media_type.c            |
| media_sink.c (MF接收器)    |                     | wg_allocator.c             |
| aac_decoder.c              |                     | wg_sample.c                |
| wma_decoder.c              |                     +----------------------------+
| video_processor.c          |
| color_convert.c            |
| resampler.c                |
| mfplat.c (MF平台集成)       |
+----------------------------+
```

**核心文件**：
- `wine/dlls/winegstreamer/main.c` - Windows 侧入口和 COM 类工厂
- `wine/dlls/winegstreamer/unixlib.c` - Unix 侧 GStreamer 初始化
- `wine/dlls/winegstreamer/wg_parser.c` - 解复用器后端
- `wine/dlls/winegstreamer/wg_transform.c` - 转码后端
- `wine/dlls/winegstreamer/wg_muxer.c` - 多路复用后端
- `wine/dlls/winegstreamer/wg_format.c` - 格式转换

### 11.2 支持的视频解码器

| 解码器 | 输入格式 | 输出格式 | 实现文件 |
|:-------|:---------|:---------|:---------|
| **H.264** | `MFVideoFormat_H264`, `MFVideoFormat_H264_ES` | NV12, YV12, IYUV, I420, YUY2 | `video_decoder.c:1695-1720` |
| **WMV** | WMV1, WMV2, WMV3, WMVA, WVC1, WMVP, WVP2, VC1S | NV12, YV12, IYUV, I420, YUY2, UYVY, RGB32, RGB24 等 | `video_decoder.c:1792-1820` |
| **Indeo 5 (IV50)** | `MFVideoFormat_IV50` | YV12, YUY2, NV11, NV12, RGB32, RGB24, RGB565, RGB555 | `video_decoder.c:1753-1770` |
| **MPEG-1/2** | `MEDIASUBTYPE_MPEG1Payload`, `MEDIASUBTYPE_MPEG2_VIDEO` | NV12 | `quartz_transform.c:941-970` |

### 11.3 支持的音频解码器

| 解码器 | 输入格式 | 输出格式 | 实现文件 |
|:-------|:---------|:---------|:---------|
| **AAC** | `WAVE_FORMAT_MPEG_HEAAC`, `WAVE_FORMAT_RAW_AAC1`, `WAVE_FORMAT_MPEG_ADTS_AAC` | Float, PCM | `aac_decoder.c:599-620` |
| **WMA** | MSAUDIO1, WMAudioV8, WMAudioV9, WMAudio_Lossless | Float, PCM | `wma_decoder.c:35-42` |
| **MP3** | `WAVE_FORMAT_MPEGLAYER3` | PCM | `quartz_transform.c:1062-1089` |
| **MPEG Audio** | `WAVE_FORMAT_MPEG` | PCM | `quartz_transform.c:804-835` |

### 11.4 支持的编码器

| 编码器 | 输入格式 | 输出格式 | 实现文件 |
|:-------|:---------|:---------|:---------|
| **H.264** | NV12, YV12, I420, YUY2 | `MFVideoFormat_H264` | `video_encoder.c:752-780` |

### 11.5 DirectShow 接口实现

**解析器 (quartz_parser.c)**：
| 组件 | 功能 | GStreamer 元素 |
|:-----|:-----|:---------------|
| **AVI Splitter** | AVI 容器解复用 | `decodebin` |
| **MPEG Splitter** | MPEG-PS/TS 解复用 | `decodebin` |
| **Wave Parser** | WAV 文件解析 | `decodebin` |
| **Decodebin Parser** | 通用解复用 | `decodebin` |

**转换器 (quartz_transform.c)**：
| 组件 | 功能 | GStreamer 元素 |
|:-----|:-----|:---------------|
| **MPEG Audio Codec** | MPEG-1/2 音频解码 | `audio/mpeg` |
| **MPEG Video Codec** | MPEG-1/2 视频解码 | `video/mpeg` |
| **MP3 Decoder** | MP3 解码 | `audio/mpeg` |
| **WMA Decoder** | WMA 解码 | `audio/x-wma` |
| **WMV Decoder** | WMV 解码 | `video/x-wmv` |

### 11.6 Media Foundation 接口实现

**源 (media_source.c)**：
- `IMFMediaSource` 接口实现
- `GStreamerByteStreamHandler` 字节流处理器
- 支持异步操作和多流管理

**接收器 (media_sink.c)**：
- `IMFFinalizableMediaSink` 接口
- 支持 MP3 和 MPEG4 输出

**变换 (Transforms)**：
- `IMFTransform` 实现在 video_decoder.c、video_encoder.c、aac_decoder.c、wma_decoder.c
- `IMediaObject` (DMO) 兼容接口

### 11.7 容器格式支持

**解复用器**：
- 使用 GStreamer 的 `decodebin` 自动插件机制
- 支持 AVI、MPEG-PS/TS、MKV、MP4、FLV、OGG 等容器
- 格式支持取决于系统安装的 GStreamer 插件

**多路复用器**：
| 输出格式 | GStreamer 格式 | 说明 |
|:---------|:---------------|:-----|
| **MP3** | `application/x-id3` | MP3 (ID3 标签) |
| **MPEG-4** | `video/quicktime, variant=iso` | MP4/MOV |

### 11.8 硬件加速支持

**当前状态**：有限支持，主要为软件解码

**D3D11 感知**：
- H.264 解码器设置 `MF_SA_D3D11_AWARE` 属性
- 支持 `CODECAPI_AVDecVideoAcceleration_H264`

**VA-API 注意事项**：
- `vaapidecodebin` 被禁用，因为它会破坏 Wine 的同步要求
- 建议直接使用 VA-API 解码器

**Fluendo 硬件加速**：
- Fluendo 硬件加速视频解码器被跳过，因为它在 Wine 中会出错

### 11.9 格式协商和转换

**GStreamer Caps <-> wg_format 转换**：
- `wg_format_from_caps()`：将 GStreamer Caps 转换为内部 wg_format
- `wg_format_to_caps()`：将 wg_format 转换回 GStreamer Caps

**Media Type 转换**：
- `caps_from_media_type()`：Windows Media Type -> GStreamer Caps
- `caps_to_media_type()`：GStreamer Caps -> Windows Media Type

**支持的格式类型**：
- `audio/x-raw`：原始 PCM 音频
- `video/x-raw`：原始视频
- `audio/mpeg`：MPEG 音频
- `audio/x-wma`：Windows Media Audio
- `video/x-wmv`：Windows Media Video
- `video/x-h264`：H.264 视频
- `video/x-cinepak`：Cinepak 视频
- `video/x-indeo`：Indeo 视频

### 11.10 注册的 COM 类

| 类名 | UUID | 功能 |
|:-----|:-----|:-----|
| `wg_avi_splitter` | `{272bfbfb-50d0-4078-...}` | AVI 分离器 |
| `wg_mpeg_audio_decoder` | `{c9f285f8-4380-4121-...}` | MPEG 音频解码 |
| `wg_mpeg_video_decoder` | `{5ed2e5f6-bf3e-4180-...}` | MPEG 视频解码 |
| `wg_mp3_decoder` | `{84cd8e3e-b221-434a-...}` | MP3 解码 |
| `wg_mpeg1_splitter` | `{a8edbf98-2442-42c5-...}` | MPEG1 分离 |
| `wg_wave_parser` | `{3f839ec7-5ea6-49e1-...}` | WAV 解析 |
| `decodebin_parser` | `{f9d8d64e-a144-47dc-...}` | 通用解码 |
| `wg_video_processor` | `{d527607f-89cb-4e94-...}` | 视频处理 |
| `GStreamerByteStreamHandler` | `{317df618-5e5a-468a-...}` | 字节流处理 |
| `wg_wma_decoder` | `{5b4d4e54-0620-4cf9-...}` | WMA 解码 |
| `wg_aac_decoder` | `{e7889a8a-2083-4844-...}` | AAC 解码 |
| `wg_wmv_decoder` | `{62ee5ddb-4f52-48e2-...}` | WMV 解码 |
| `wg_h264_decoder` | `{1f1e273d-12c0-4b3a-...}` | H.264 解码 |
| `wg_h264_encoder` | `{6c34de69-4670-46cd-...}` | H.264 编码 |
| `wg_resampler` | `{92f35e78-15a5-486b-...}` | 音频重采样 |
| `wg_color_converter` | `{f47e2da5-e370-47b7-...}` | 颜色转换 |
| `wg_mp3_sink_factory` | `{1f302877-aaab-40a3-...}` | MP3 输出 |
| `wg_mpeg4_sink_factory` | `{5d5407d9-c6ca-4770-...}` | MPEG4 输出 |

### 11.11 辅助处理器

**音频重采样器 (resampler.c)**：
- 输入/输出格式：`MFAudioFormat_Float` (32-bit), `MFAudioFormat_PCM` (16-bit)
- 功能：采样率转换、通道数转换、位深度转换
- 实现：使用 GStreamer `audioconvert` 和 `audioresample` 元素

**颜色空间转换器 (color_convert.c)**：
- 输入格式：YV12, YUY2, UYVY, AYUV, NV12, RGB32, RGB565, I420, IYUV, YVYU, RGB24, RGB555, NV11 等
- 输出格式：YV12, YUY2, UYVY, AYUV, NV12, RGB32, RGB565, I420, IYUV, YVYU, RGB24, RGB555, NV11
- 实现：使用 GStreamer `videoconvert` 和 `videoscale` 元素

**视频处理器 (video_processor.c)**：
- 输入格式：IYUV, YV12, NV12, 420O, UYVY, YUY2, NV11, AYUV, ARGB32, RGB32, RGB24, I420, YVYU, RGB555, RGB565, RGB8 等
- 输出格式：YUY2, IYUV, I420, NV12, RGB24, ARGB32, RGB32, YV12, AYUV, RGB555, RGB565, ABGR32

### 11.12 自定义 GStreamer 元素

**WgStepper** (`wg_transform.c:60-80`)：
- 功能：从 sink pad 接收缓冲区和事件，保留在内部队列中
- 用途：控制转码管道的数据流，支持同步 drain/flush 操作

---

## 12. winedmo vs winegstreamer 对比

| 特性 | winedmo | winegstreamer |
|:-----|:--------|:--------------|
| **后端** | FFmpeg | GStreamer |
| **绕过 GStreamer** | ✅ 是 | ❌ 否 |
| **DirectShow 支持** | ❌ 否 | ✅ 是 |
| **Media Foundation 支持** | ✅ 是 | ✅ 是 |
| **容器格式** | MP4, AVI, WAV, ASF, WMV, WMA, MPEG, MP3 | 所有 GStreamer 支持的格式 |
| **视频编解码器** | H.264, MPEG-1, VP9, 原始视频 | H.264, WMV, MPEG-1/2, Indeo 5, Cinepak |
| **音频编解码器** | AAC, MP1, MP3, WMA v1/v2/Pro/Voice/Lossless, PCM, IEEE Float | AAC, WMA, MP3, MPEG Audio |
| **硬件加速** | 通过 FFmpeg 硬件加速 | 有限（VA-API 被禁用） |
| **编码支持** | ❌ 否（仅解复用） | ✅ 是（H.264 编码） |
| **外部依赖** | FFmpeg 库 | GStreamer 库 |
| **配置控制** | `DisableGstByteStreamHandler` 注册表 | 无（默认使用） |

### 12.1 使用场景选择

**使用 winedmo 的场景**：
1. 需要绕过 GStreamer 框架
2. 系统未安装 GStreamer 或插件不完整
3. 需要更好的 FFmpeg 格式支持
4. 避免 GStreamer 的线程模型问题

**使用 winegstreamer 的场景**：
1. 需要 DirectShow 接口支持
2. 需要编码功能（如 H.264 编码）
3. 需要更广泛的容器格式支持
4. 系统已安装完整的 GStreamer 插件

### 12.2 配置选项

**强制使用 winedmo（绕过 GStreamer）**：
```
注册表路径: HKEY_CURRENT_USER\Software\Wine\MediaFoundation
键名: DisableGstByteStreamHandler
类型: DWORD
值: 1
```

**注意事项**：
- 该配置仅对 Media Foundation 字节流处理器有效
- DirectShow 路径不受此配置影响
- 如果 FFmpeg 不支持某种格式，仍会回退到 GStreamer
