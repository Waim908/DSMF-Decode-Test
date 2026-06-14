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
