# Android Rtmp 播放器
oarplayer(only android rtmp player)是一款简单到毫无特色的纯rtmp播放器,
不依赖ffmpeg,仅依赖srs-librtmp第三方库,体积小,可调整性强.

## 功能介绍
- 未依赖ffmpeg框架,基于srs-librtmp的rtmp拉流,编译打包更简单;
- 支持Android API level 16及以上系统;
- 完全使用Android硬件解码,支持FLV封装的H264+AAC音视频编码直播;
- Android API level 21以下使用java层创建的Mediacodec解码,API level21及以上使用NdkMediaCodec,解码不需要与java交互,效率更高;
- 视频显示使用MediaCodec(Android 硬解)->SurfaceTexture->OpenGL ES工作模式,性能优异;
- 音频播放使用OpenSL ES,直接native层读取音频解码帧播放;
- 支持后台播放(只播放声音);
- 支持设置缓存时长.

## 已知问题及优化计划
[ x ] srs-librtmp库裁剪以及增加读超时时间配置接口;
[ x ] 音频未支持重采样,对于部分对声音播放采样率有要求的手机支持不好;
[ x ] x264以及libaac软解码库支持(未确定);
[ x ] 视频绘制滤镜增加(配置接口与各滤镜效果shader);
[ x ] 实时截图功能;
[ x ] 视频边看边下载功能;
[ x ] 播放统计


## 使用说明
gradle中导入:
```
compile 'com.github.qingkouwei:oarplayer:0.0.1-SNAPSHOT'
```
通过以下几步即可完成rtmp播放:

1. 实例化OARPlayer:`OARPlayer player = new OARPlayer();`
2. 设置视频源:`player.setDataSource(rtmp_url);`
3. 设置surface:`player.setSurface(surfaceView.getHolder());`
4. 开始播放:`player.start();`
5. 停止播放:`player.stop();`
6. 释放资源:`player.release();`
## 联系方式
email:turn.shen@gmail.com

OARPlayer is licenced under [MIT](https://github.com/qingkouwei/oarplayer/blob/master/LICENSE).
**欢迎使用,欢迎star**
