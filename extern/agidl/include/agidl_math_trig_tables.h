#ifndef TRIG_TABLES_H
#define TRIG_TABLES_H

/********************************************
*   Adaptive Graphics Image Display Library
*
*   Copyright (c) 2023-2024 Ryandracus Chapman
*
*   Library: libagidl
*   File: agidl_math_trig_tables.h
*   Date: 12/9/2023
*   Version: 0.1b
*   Updated: 1/19/2024
*   Author: Ryandracus Chapman
*
********************************************/

/**
	Look-up table(LUT) for the radian values of the trig function sin
	with each index of the array representing a degree value.
*/
const float sin_lut[] = {
0.0000f,0.0175f,0.0349f,0.0523f,0.0698f,0.0872f,0.1045f,0.1219f,0.1392f,0.1564f,0.1736f,0.1908f,0.2079f,0.2250f,0.2419f,0.2588f,0.2756f,0.2924f,0.3090f,0.3256f,0.3420f,
0.3584f,0.3746f,0.3907f,0.4067f,0.4226f,0.4384f,0.4540f,0.4695f,0.4848f,0.5000f,0.5150f,0.5299f,0.5446f,0.5592f,0.5736f,0.5878f,0.6018f,0.6157f,0.6293f,0.6428f,
0.6561f,0.6691f,0.6820f,0.6947f,0.7071f,0.7193f,0.7314f,0.7431f,0.7547f,0.7660f,0.7771f,0.7880f,0.7986f,0.8090f,0.8192f,0.8290f,0.8387f,0.8480f,0.8572f,0.8660f,
0.8746f,0.8829f,0.8910f,0.8988f,0.9063f,0.9135f,0.9205f,0.9272f,0.9336f,0.9397f,0.9455f,0.9511f,0.9563f,0.9613f,0.9659f,0.9703f,0.9744f,0.9781f,0.9816f,0.9848f,
0.9877f,0.9903f,0.9925f,0.9945f,0.9962f,0.9976f,0.9986f,0.9994f,0.9998f,1.0000f,0.9998f,0.9994f,0.9986f,0.9976f,0.9962f,0.9945f,0.9925f,0.9903f,0.9877f,0.9848f,
0.9816f,0.9781f,0.9744f,0.9703f,0.9659f,0.9613f,0.9563f,0.9511f,0.9455f,0.9397f,0.9336f,0.9272f,0.9205f,0.9135f,0.9063f,0.8988f,0.8910f,0.8829f,0.8746f,0.8660f,
0.8572f,0.8480f,0.8387f,0.8290f,0.8192f,0.8090f,0.7986f,0.7880f,0.7771f,0.7660f,0.7547f,0.7431f,0.7314f,0.7193f,0.7071f,0.6947f,0.6820f,0.6691f,0.6561f,0.6428f,
0.6293f,0.6157f,0.6018f,0.5878f,0.5736f,0.5592f,0.5446f,0.5299f,0.5150f,0.5000f,0.4848f,0.4695f,0.4540f,0.4384f,0.4226f,0.4067f,0.3907f,0.3746f,0.3584f,0.3420f,
0.3256f,0.3090f,0.2924f,0.2756f,0.2588f,0.2419f,0.2250f,0.2079f,0.1908f,0.1736f,0.1564f,0.1392f,0.1219f,0.1045f,0.0872f,0.0698f,0.0523f,0.0349f,0.0175f,0.0000f,
-0.0175f,-0.0349f,-0.0523f,-0.0698f,-0.0872f,-0.1045f,-0.1219f,-0.1392f,-0.1564f,-0.1736f,-0.1908f,-0.2079f,-0.2250f,-0.2419f,-0.2588f,-0.2756f,-0.2924f,-0.3090f,-0.3256f,-0.3420f,
-0.3584f,-0.3746f,-0.3907f,-0.4067f,-0.4226f,-0.4384f,-0.4540f,-0.4695f,-0.4848f,-0.5000f,-0.5150f,-0.5299f,-0.5446f,-0.5592f,-0.5736f,-0.5878f,-0.6018f,-0.6157f,-0.6293f,-0.6428f,
-0.6561f,-0.6691f,-0.6820f,-0.6947f,-0.7071f,-0.7193f,-0.7314f,-0.7431f,-0.7547f,-0.7660f,-0.7771f,-0.7880f,-0.7986f,-0.8090f,-0.8192f,-0.8290f,-0.8387f,-0.8480f,-0.8572f,-0.8660f,
-0.8746f,-0.8829f,-0.8910f,-0.8988f,-0.9063f,-0.9135f,-0.9205f,-0.9272f,-0.9336f,-0.9397f,-0.9455f,-0.9511f,-0.9563f,-0.9613f,-0.9659f,-0.9703f,-0.9744f,-0.9781f,-0.9816f,-0.9848f,
-0.9877f,-0.9903f,-0.9925f,-0.9945f,-0.9962f,-0.9976f,-0.9986f,-0.9994f,-0.9998f,-1.0000f,-0.9998f,-0.9994f,-0.9986f,-0.9976f,-0.9962f,-0.9945f,-0.9925f,-0.9903f,-0.9877f,-0.9848f,
-0.9816f,-0.9781f,-0.9744f,-0.9703f,-0.9659f,-0.9613f,-0.9563f,-0.9511f,-0.9455f,-0.9397f,-0.9336f,-0.9272f,-0.9205f,-0.9135f,-0.9063f,-0.8988f,-0.8910f,-0.8829f,-0.8746f,-0.8660f,
-0.8572f,-0.8480f,-0.8387f,-0.8290f,-0.8192f,-0.8090f,-0.7986f,-0.7880f,-0.7771f,-0.7660f,-0.7547f,-0.7431f,-0.7314f,-0.7193f,-0.7071f,-0.6947f,-0.6820f,-0.6691f,-0.6561f,-0.6428f,
-0.6293f,-0.6157f,-0.6018f,-0.5878f,-0.5736f,-0.5592f,-0.5446f,-0.5299f,-0.5150f,-0.5000f,-0.4848f,-0.4695f,-0.4540f,-0.4384f,-0.4226f,-0.4067f,-0.3907f,-0.3746f,-0.3584f,-0.3420f,
-0.3256f,-0.3090f,-0.2924f,-0.2756f,-0.2588f,-0.2419f,-0.2250f,-0.2079f,-0.1908f,-0.1736f,-0.1564f,-0.1392f,-0.1219f,-0.1045f,-0.0872f,-0.0698f,-0.0523f,-0.0349f,-0.0175f,-0.0000f
};
 /**
  Look-up table(LUT) for the radian values of the trig function cos
  with each index of the array representing a degree value.
*/
const float cos_lut[] = {
 1.000f,0.9998f,0.9994f,0.9986f,0.9976f,0.9962f,0.9945f,0.9925f,0.9903f,0.9877f,0.9848f,0.9816f,0.9781f,0.9744f,0.9703f,0.9659f,0.9613f,0.9563f,0.9511f,0.9455f,0.9397f,
 0.9336f,0.9272f,0.9205f,0.9135f,0.9063f,0.8988f,0.8910f,0.8829f,0.8746f,0.8660f,0.8572f,0.8480f,0.8387f,0.8290f,0.8192f,0.8090f,0.7986f,0.7880f,0.7771f,0.7660f,
 0.7547f,0.7431f,0.7314f,0.7193f,0.7071f,0.6947f,0.6820f,0.6691f,0.6561f,0.6428f,0.6293f,0.6157f,0.6018f,0.5878f,0.5736f,0.5592f,0.5446f,0.5299f,0.5150f,0.5000f,
 0.4848f,0.4695f,0.4540f,0.4384f,0.4226f,0.4067f,0.3907f,0.3746f,0.3584f,0.3420f,0.3256f,0.3090f,0.2924f,0.2756f,0.2588f,0.2419f,0.2250f,0.2079f,0.1908f,0.1736f,
 0.1564f,0.1392f,0.1219f,0.1045f,0.0872f,0.0698f,0.0523f,0.0349f,0.0175f,0.0000f,-0.0175f,-0.0349f,-0.0523f,-0.0698f,-0.0872f,-0.1045f,-0.1219f,-0.1392f,-0.1564f,-0.1736f,
-0.1908f,-0.2079f,-0.2250f,-0.2419f,-0.2588f,-0.2756f,-0.2924f,-0.3090f,-0.3256f,-0.3420f,-0.3584f,-0.3746f,-0.3907f,-0.4067f,-0.4226f,-0.4384f,-0.4540f,-0.4695f,-0.4848f,-0.5000f,
-0.5150f,-0.5299f,-0.5446f,-0.5592f,-0.5736f,-0.5878f,-0.6018f,-0.6157f,-0.6293f,-0.6428f,-0.6561f,-0.6691f,-0.6820f,-0.6947f,-0.7071f,-0.7193f,-0.7314f,-0.7431f,-0.7547f,-0.7660f,
-0.7771f,-0.7880f,-0.7986f,-0.8090f,-0.8192f,-0.8290f,-0.8387f,-0.8480f,-0.8572f,-0.8660f,-0.8746f,-0.8829f,-0.8910f,-0.8988f,-0.9063f,-0.9135f,-0.9205f,-0.9272f,-0.9336f,-0.9397f,
-0.9455f,-0.9511f,-0.9563f,-0.9613f,-0.9659f,-0.9703f,-0.9744f,-0.9781f,-0.9816f,-0.9848f,-0.9877f,-0.9903f,-0.9925f,-0.9945f,-0.9962f,-0.9976f,-0.9986f,-0.9994f,-0.9998f,-1.0000f,
-0.9998f,-0.9994f,-0.9986f,-0.9976f,-0.9962f,-0.9945f,-0.9925f,-0.9903f,-0.9877f,-0.9848f,-0.9816f,-0.9781f,-0.9744f,-0.9703f,-0.9659f,-0.9613f,-0.9563f,-0.9511f,-0.9455f,-0.9397f,
-0.9336f,-0.9272f,-0.9205f,-0.9135f,-0.9063f,-0.8988f,-0.8910f,-0.8829f,-0.8746f,-0.8660f,-0.8572f,-0.8480f,-0.8387f,-0.8290f,-0.8192f,-0.8090f,-0.7986f,-0.7880f,-0.7771f,-0.7660f,
-0.7547f,-0.7431f,-0.7314f,-0.7193f,-0.7071f,-0.6947f,-0.6820f,-0.6691f,-0.6561f,-0.6428f,-0.6293f,-0.6157f,-0.6018f,-0.5878f,-0.5736f,-0.5592f,-0.5446f,-0.5299f,-0.5150f,-0.5000f,
-0.4848f,-0.4695f,-0.4540f,-0.4384f,-0.4226f,-0.4067f,-0.3907f,-0.3746f,-0.3584f,-0.3420f,-0.3256f,-0.3090f,-0.2924f,-0.2756f,-0.2588f,-0.2419f,-0.2250f,-0.2079f,-0.1908f,-0.1736f,
-0.1564f,-0.1392f,-0.1219f,-0.1045f,-0.0872f,-0.0698f,-0.0523f,-0.0349f,-0.0175f,-0.0000f,0.0175f,0.0349f,0.0523f,0.0698f,0.0872f,0.1045f,0.1219f,0.1392f,0.1564f,0.1736f,
 0.1908f,0.2079f,0.2250f,0.2419f,0.2588f,0.2756f,0.2924f,0.3090f,0.3256f,0.3420f,0.3584f,0.3746f,0.3907f,0.4067f,0.4226f,0.4384f,0.4540f,0.4695f,0.4848f,0.5000f,
 0.5150f,0.5299f,0.5446f,0.5592f,0.5736f,0.5878f,0.6018f,0.6157f,0.6293f,0.6428f,0.6561f,0.6691f,0.6820f,0.6947f,0.7071f,0.7193f,0.7314f,0.7431f,0.7547f,0.7660f,
 0.7771f,0.7880f,0.7986f,0.8090f,0.8192f,0.8290f,0.8387f,0.8480f,0.8572f,0.8660f,0.8746f,0.8829f,0.8910f,0.8988f,0.9063f,0.9135f,0.9205f,0.9272f,0.9336f,0.9397f,
 0.9455f,0.9511f,0.9563f,0.9613f,0.9659f,0.9703f,0.9744f,0.9781f,0.9816f,0.9848f,0.9877f,0.9903f,0.9925f,0.9945f,0.9962f,0.9976f,0.9986f,0.9994f,0.9998f,1.0000f
};
 /**
  Look-up table(LUT) for the radian values of the trig function tan
  with each index of the array representing a degree value.
*/
const float tan_lut[] = {
 0.0000f,0.0175f,0.0349f,0.0524f,0.0699f,0.0875f,0.1051f,0.1228f,0.1405f,0.1584f,0.1763f,0.1944f,0.2126f,0.2309f,0.2493f,0.2679f,0.2867f,0.3057f,0.3249f,0.3443f,0.3640f,
 0.3839f,0.4040f,0.4245f,0.4452f,0.4663f,0.4877f,0.5095f,0.5317f,0.5543f,0.5774f,0.6009f,0.6249f,0.6494f,0.6745f,0.7002f,0.7265f,0.7536f,0.7813f,0.8098f,0.8391f,
 0.8693f,0.9004f,0.9325f,0.9657f,1.0000f,1.0355f,1.0724f,1.1106f,1.1504f,1.1918f,1.2349f,1.2799f,1.3270f,1.3764f,1.4281f,1.4826f,1.5399f,1.6003f,1.6643f,1.7321f,
 1.8040f,1.8807f,1.9626f,2.0503f,2.1445f,2.2460f,2.3559f,2.4751f,2.6051f,2.7475f,2.9042f,3.0777f,3.2709f,3.4874f,3.7321f,4.0108f,4.3315f,4.7046f,5.1446f,5.6713f,
 6.3138f,7.1154f,8.1443f,9.5144f,11.4300f,14.3007f,19.0811f,28.6362f,57.2899f,37320539.6344f,-57.2901f,-28.6363f,-19.0811f,-14.3007f,-11.4301f,-9.5144f,-8.1443f,-7.1154f,-6.3138f,-5.6713f,
-5.1446f,-4.7046f,-4.3315f,-4.0108f,-3.7321f,-3.4874f,-3.2709f,-3.0777f,-2.9042f,-2.7475f,-2.6051f,-2.4751f,-2.3559f,-2.2460f,-2.1445f,-2.0503f,-1.9626f,-1.8807f,-1.8040f,-1.7321f,
-1.6643f,-1.6003f,-1.5399f,-1.4826f,-1.4281f,-1.3764f,-1.3270f,-1.2799f,-1.2349f,-1.1918f,-1.1504f,-1.1106f,-1.0724f,-1.0355f,-1.0000f,-0.9657f,-0.9325f,-0.9004f,-0.8693f,-0.8391f,
-0.8098f,-0.7813f,-0.7536f,-0.7265f,-0.7002f,-0.6745f,-0.6494f,-0.6249f,-0.6009f,-0.5774f,-0.5543f,-0.5317f,-0.5095f,-0.4877f,-0.4663f,-0.4452f,-0.4245f,-0.4040f,-0.3839f,-0.3640f,
-0.3443f,-0.3249f,-0.3057f,-0.2867f,-0.2679f,-0.2493f,-0.2309f,-0.2126f,-0.1944f,-0.1763f,-0.1584f,-0.1405f,-0.1228f,-0.1051f,-0.0875f,-0.0699f,-0.0524f,-0.0349f,-0.0175f,-0.0000f,
0.0175f,0.0349f,0.0524f,0.0699f,0.0875f,0.1051f,0.1228f,0.1405f,0.1584f,0.1763f,0.1944f,0.2126f,0.2309f,0.2493f,0.2679f,0.2867f,0.3057f,0.3249f,0.3443f,0.3640f,
0.3839f,0.4040f,0.4245f,0.4452f,0.4663f,0.4877f,0.5095f,0.5317f,0.5543f,0.5774f,0.6009f,0.6249f,0.6494f,0.6745f,0.7002f,0.7265f,0.7536f,0.7813f,0.8098f,0.8391f,
0.8693f,0.9004f,0.9325f,0.9657f,1.0000f,1.0355f,1.0724f,1.1106f,1.1504f,1.1918f,1.2349f,1.2799f,1.3270f,1.3764f,1.4281f,1.4826f,1.5399f,1.6003f,1.6643f,1.7321f,
1.8040f,1.8807f,1.9626f,2.0503f,2.1445f,2.2460f,2.3559f,2.4751f,2.6051f,2.7475f,2.9042f,3.0777f,3.2709f,3.4874f,3.7320f,4.0108f,4.3315f,4.7046f,5.1446f,5.6713f,
6.3137f,7.1154f,8.1443f,9.5144f,11.4300f,14.3006f,19.0811f,28.6362f,57.2897f,12440179.8094f,-57.2902f,-28.6363f,-19.0812f,-14.3007f,-11.4301f,-9.5144f,-8.1444f,-7.1154f,-6.3138f,-5.6713f,
-5.1446f,-4.7046f,-4.3315f,-4.0108f,-3.7321f,-3.4874f,-3.2709f,-3.0777f,-2.9042f,-2.7475f,-2.6051f,-2.4751f,-2.3559f,-2.2460f,-2.1445f,-2.0503f,-1.9626f,-1.8807f,-1.8040f,-1.7321f,
-1.6643f,-1.6003f,-1.5399f,-1.4826f,-1.4281f,-1.3764f,-1.3270f,-1.2799f,-1.2349f,-1.1918f,-1.1504f,-1.1106f,-1.0724f,-1.0355f,-1.0000f,-0.9657f,-0.9325f,-0.9004f,-0.8693f,-0.8391f,
-0.8098f,-0.7813f,-0.7536f,-0.7265f,-0.7002f,-0.6745f,-0.6494f,-0.6249f,-0.6009f,-0.5774f,-0.5543f,-0.5317f,-0.5095f,-0.4877f,-0.4663f,-0.4452f,-0.4245f,-0.4040f,-0.3839f,-0.3640f,
-0.3443f,-0.3249f,-0.3057f,-0.2867f,-0.2679f,-0.2493f,-0.2309f,-0.2126f,-0.1944f,-0.1763f,-0.1584f,-0.1405f,-0.1228f,-0.1051f,-0.0875f,-0.0699f,-0.0524f,-0.0349f,-0.0175f,-0.0000f
};

#endif
