### ffmpeg  使用  
- 推流 
  ffmpeg.exe -re -i c:/video/1.mp4 -c copy -f flv rtmp://localhost:1935/live/movie   
   ffmpeg.exe -re -i c:/video/1.mp4 -c copy -f flv rtmp://192.168.43.146:1935/live/movie   
### HLS 拉流 
http://127.0.0.1:7002/live/movie.m3u8 
http://192.168.43.146:7002/live/movie.m3u8

### HTML 中显示编码 
``` 
调试通过，使用video.js 正确才能正常的播放  
 <link href="https://unpkg.com/video.js/dist/video-js.css" rel="stylesheet">   
  <script src="https://unpkg.com/video.js/dist/video.js"></script>    
 <script src="https://unpkg.com/videojs-contrib-hls/dist/videojs-contrib-hls.js"></script>
 
 	<video id="my_video_1" class="video-js vjs-default-skin" controls preload="auto" width="500" height="500" data-setup='{}'>
    <source src="http://192.168.43.146:7002/live/movie.m3u8" type="application/x-mpegURL"> 
  	</video> 

```  
### 由于没有引用video-contrib-hls.js 播放失败，no compatible 
``` 
<link href="//vjs.zencdn.net/4.9/video-js.css" rel="stylesheet">
<script src="//vjs.zencdn.net/4.9/video.js"></script> 
``` 

### 播放一段时间后， 崩溃 



