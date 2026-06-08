# 'DSMF-Decode-Test' is DirectShow MediaFoundation Decode Test

通过windows api解码视频与音频。

注意！此软件不能真实反映系统的实际解码效果，其次某些的系统肯定装了第三方解码器这些，其次windows的directshow mf框架实际支持的格式实在有限，很多视频可能都无法播放！

对于Wine，可能大多数视频与音频都交给gstreamer框架进行处理，反而能播放几乎任何视频--只要你gstreamer插件集合足够完整，由于mf框架方面，Proton分支的Wine的MF框架与原版Wine存在巨大差异，因此反而更可能出问题！


# 参考资料

- https://github.com/microsoft/media-foundation.git

- https://github.com/MicrosoftDocs/win32/tree/docs/desktop-src/DirectShow

- https://github.com/MicrosoftDocs/win32/tree/docs/desktop-src/medfound
