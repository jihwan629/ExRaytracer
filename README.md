# OpenGL Ray Tracing 구현 프로젝트


<br>

> [Visual Studio에서 OpenGL 설치하기](https://blog.amaorche.com/20)  <br>
> 3차원 좌표와 벡터를 저장, 연산자 오버로딩을 구현한 gmath.h, gmath.cpp는 직접 작성한 코드가 아닙니다. <br>


<br>
<br>

 ## Ray Tracing (광선 추적)

 <img src="https://developer.nvidia.com/sites/default/files/pictures/2018/RayTracing/ray-tracing-image-1.jpg" width="500"/>
 
 <br>
 
[사진 출처](https://developer.nvidia.com/discover/ray-tracing)

<br>
<br>

> 가상 광선이 물체의 표면에서 반사되어, 카메라를 거쳐 다시 돌아오는 경로를 계산하는 것 <br>


<br>
<br>

 ## 결과
 
 <img src="https://user-images.githubusercontent.com/31186176/104588522-d5b90180-56ab-11eb-9a57-a98b45b6660b.png" width="500"/>
 https://youtu.be/OLPNIr7x6EE
 
<br>
<br>
 
 ## 과정
 
 <br>
 
 - **광선과 구의 교차점**
 
 <br>
 
  <img src="https://user-images.githubusercontent.com/31186176/104585810-f5e6c180-56a7-11eb-818f-ae6d7d9a6bad.png" width="500"/>
  
  <br>
  
  <img src="https://user-images.githubusercontent.com/53321189/88262622-7f25c880-cd03-11ea-896e-0e89ab685538.png" width="500"/>
  
  [사진 출처](https://yeosong1.github.io/rt-%EA%B4%91%EC%84%A0-%EA%B5%AC-%EA%B5%90%EC%B0%A8)
 
 <br>
 <br>
 
 - **반사**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104586451-d43a0a00-56a8-11eb-9b39-dfe169edf74c.png" width="500"/>
 
  <br>
  
  <img src="https://user-images.githubusercontent.com/31186176/104585639-a902eb00-56a7-11eb-931d-4c4fe8d53c7b.png"/>
 
 <br>
 <br>
 
 - **굴절**
 
 <br>
 
 <img src="https://user-images.githubusercontent.com/31186176/104585687-c0da6f00-56a7-11eb-8437-e3eac1bd4bda.png" width="500"/>
 
 > 굴절 광선은 구 안으로 들어올 때, 구 밖으로 나갈 때 모두 2번 구해야한다. <br>
 
 <br>
 <br>
 
 - **퐁 쉐이딩**
 
 <br>

<img src="https://user-images.githubusercontent.com/31186176/104586791-4d396180-56a9-11eb-9b34-8f1aac766901.png" width="500"/>
<img src="https://user-images.githubusercontent.com/31186176/104586723-31ce5680-56a9-11eb-8b95-bd8b90fb5e3f.png" width="500"/>
 
 <br>
 <br>
 
