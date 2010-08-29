clear all; clc; clear global; close all;
format long;

%% C-code to compare against

% // *************************************** //
% // **** TEMP CODE to test matrix code **** //
% // *************************************** //
% // Correct results are in "MatlabTests.m"
% 	double3x3 M1, M2, res;
% 	double3 vec1,res1;
% 	M1.x.x = 1;  M1.x.y = 2;  M1.x.z = 3; 
% 	M1.y.x = 4;  M1.y.y = 5;  M1.y.z = 6; 
% 	M1.z.x = 7;  M1.z.y = 8;  M1.z.z = 9; 
% 	M2.x.x = 10; M2.x.y = 11; M2.x.z = 12; 
% 	M2.y.x = 13; M2.y.y = 14; M2.y.z = 15; 
% 	M2.z.x = 16; M2.z.y = 17; M2.z.z = 18; 
% 	vec1.x = 19; vec1.y = 20; vec1.z = 21;
% 
% 	MatrixMult( & res, & M1, & M2 );			// M1 * M2
% 	MatrixMult_A_BTran( & res, & M1, & M2 );    // M1 * M2^t
% 	MatrixMult_ATran_B( & res, & M1, & M2 );    // M1^t * M2
% 	MatrixMult( & res1, & M1, & vec1 );			// M1 * vec1
% 	MatrixMult_ATran_B( & res1, & M1, & vec1 );	// M1^t * vec1
% 
% 	D3DXMATRIX M3,M4;
% 	M3._11 = 22; M3._12 = 23; M3._13 = 24; M3._14 = 0; 
% 	M3._21 = 25; M3._22 = 26; M3._23 = 27; M3._24 = 0;  
% 	M3._31 = 28; M3._32 = 29; M3._33 = 30; M3._34 = 0; 
% 	M3._41 =  0; M3._42 =  0; M3._43 =  0; M3._44 = 1; 
% 
% 	M4._11 = 31; M4._12 = 32; M4._13 = 33; M4._14 = 0; 
% 	M4._21 = 34; M4._22 = 35; M4._23 = 36; M4._24 = 0;  
% 	M4._31 = 37; M4._32 = 38; M4._33 = 39; M4._34 = 0; 
% 	M4._41 =  0; M4._42 =  0; M4._43 =  0; M4._44 = 1; 
% 
% 	MatrixMult( & res, & M1, & M3 );			// M1 * M3
% 	MatrixMult( & res, & M3, & M1 );			// M3 * M1
% 	MatrixMult_A_BTran( & res, & M1, & M3 );    // M1 * M3^t
% 	MatrixMult_A_BTran( & res, & M3, & M1 );    // M3 * M1^t
% 	MatrixMult_ATran_B( & res, & M1, & M3 );    // M1^t * M3
% 	MatrixMult_ATran_B( & res, & M3, & M1 );    // M3^t * M1
% 	MatrixMult_ATran_B( & res, & M3, & M4 );    // M3^t * M4
% 	MatrixMult_ATran_B( & res1, & M3, & vec1 );	// M3^t * vec1
% 
% 	MatrixMult_ATran_BTran( & res, & M1, & M3 );    // M1^t * M3^t
% 	MatrixMult_ATran_BTran( & res, & M3, & M1 );    // M3^t * M1^t
% 
% 	// Test cases
% 	D3DXMatrixIdentity( & a->rbo->R );
% 	D3DXMatrixRotationX(& a->rbo->R, D3DX_PI / 4 ); // Rotate 45 deg
% 	D3DXMatrixIdentity( & b->rbo->R );
% 	D3DXMatrixRotationX(& b->rbo->R, D3DX_PI / 4 ); // Rotate 45 deg
% 	a->rbo->x.x = 0.0; a->rbo->x.y = 2.40; a->rbo->x.z = 0.0;
% 	b->rbo->x.x = 0.0; b->rbo->x.y = 0.0; b->rbo->x.z = 0.0;
% 	a->rbo->scale = 1.0f;
% 	b->rbo->scale = 1.0f;
% 	a->boxCenterObjectCoord = double3(0,0,0);
% 	b->boxCenterObjectCoord = double3(0,0,0);
% 	a->orientMat = double3x3(double3(1,0,0),double3(0,1,0),double3(0,0,1));
% 	b->orientMat = double3x3(double3(1,0,0),double3(0,1,0),double3(0,0,1));
% 	a->boxDimension = double3(1,1,1);
% 	b->boxDimension = double3(1,1,1);
% 
% 
% // *************************************** //
% // **** END TEMP CODE ******************** //
% // *************************************** //

%% Inputs
EPSILON = 0.000001;

a_orientMat = [[0,-1,0];[-1,0,0];[0,0,1]];
a_rbo_R = [[-0.99999976,0,0];[0,1,0];[0,0,-0.99999976]];
b_orientMat = [[0,0,1];[-1,0,0];[0,1,0]];
b_rbo_R = [[-0.39722300,0.60277611,0.69200748];[-0.60277671,0.39722389,-0.69200671];[-0.69200701,-0.69200718,0.20555311]];
a_boxCenterObjectCoord = [-0.99999994039535522;0;0];
b_boxCenterObjectCoord = [0;0;0.99999994039535522];
a_rbo_x = [0;0;0];
a_rbo_scale = 1.0;
b_rbo_x = [0;2.3110094;0];
b_rbo_scale = 1.0;
a_boxDimension = [0.99999994039535522;1.0000000000000001e-005;0.99999994039535522];
b_boxDimension = [0.99999994039535522;1.0000000000000001e-005;0.99999994039535522];

%% Code here on: obbox.cpp: TestOBBCollision()
R_a = a_orientMat * a_rbo_R;
R_b = b_orientMat * b_rbo_R;
R = R_a * R_b'; % A_Transpose & B

t_boxCenter_a = a_boxCenterObjectCoord * a_rbo_scale;
t_boxCenter_b = b_boxCenterObjectCoord * b_rbo_scale;
t_boxCenter_a = a_rbo_R * t_boxCenter_a;
t_boxCenter_b = b_rbo_R * t_boxCenter_b;

t_world_a = a_rbo_x + t_boxCenter_a;
t_world_b = b_rbo_x + t_boxCenter_b;
t_world = t_world_b - t_world_a;

t = R_a' * t_world;

a_OBBDim = a_boxDimension .* a_rbo_scale;
b_OBBDim = b_boxDimension .* b_rbo_scale;

AbsR = abs(R) + EPSILON;

contact = 1; 

for i = 1:3
    ra = a_OBBDim(i);
    rb = (b_OBBDim(1) * AbsR(i,1)) +  (b_OBBDim(2) * AbsR(i,2)) +  (b_OBBDim(3) * AbsR(i,3));
    if(abs(t(i)) > ra + rb); disp('NO CONTACT'); contact = 0; end
end

for i = 1:3
    ra = (a_OBBDim(1) * AbsR(1,i)) + (a_OBBDim(2) * AbsR(2,i)) + (a_OBBDim(3) * AbsR(3,i));
    rb = b_OBBDim(i);
    if(abs(t(i)) > ra + rb); disp('NO CONTACT'); contact = 0; end
end

ra = a_OBBDim(2) * AbsR(3,1) + a_OBBDim(3) * AbsR(2,1);
rb = b_OBBDim(2) * AbsR(1,3) + b_OBBDim(3) * AbsR(1,2);
if (abs(t(3) * R(2,1) - t(2) * R(3,1)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(2) * AbsR(3,1) + a_OBBDim(3) * AbsR(2,2);
rb = b_OBBDim(1) * AbsR(1,3) + b_OBBDim(3) * AbsR(1,1);
if (abs(t(3) * R(2,2) - t(2) * R(3,2)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(2) * AbsR(3,3) + a_OBBDim(3) * AbsR(2,3);
rb = b_OBBDim(1) * AbsR(1,2) + b_OBBDim(2) * AbsR(1,1);
if (abs(t(3) * R(2,3) - t(2) * R(3,3)) > ra + rb); contact = 0;  disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(3,1) + a_OBBDim(3) * AbsR(1,1);
rb = b_OBBDim(2) * AbsR(2,3) + b_OBBDim(3) * AbsR(2,2);
if (abs(t(1) * R(3,1) - t(3) * R(1,1)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(3,2) + a_OBBDim(3) * AbsR(1,2);
rb = b_OBBDim(1) * AbsR(2,3) + b_OBBDim(3) * AbsR(2,1);
if (abs(t(1) * R(3,2) - t(3) * R(1,2)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(3,3) + a_OBBDim(3) * AbsR(1,3);
rb = b_OBBDim(1) * AbsR(2,2) + b_OBBDim(2) * AbsR(2,1);
if (abs(t(1) * R(3,3) - t(3) * R(1,3)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(2,1) + a_OBBDim(2) * AbsR(1,1);
rb = b_OBBDim(2) * AbsR(3,3) + b_OBBDim(3) * AbsR(3,2);
if (abs(t(2) * R(1,1) - t(1) * R(2,1)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(2,2) + a_OBBDim(2) * AbsR(1,2);
rb = b_OBBDim(1) * AbsR(3,3) + b_OBBDim(3) * AbsR(3,1);
if (abs(t(2) * R(1,2) - t(1) * R(2,2)) > ra + rb); contact = 0; disp('NO CONTACT'); end

ra = a_OBBDim(1) * AbsR(2,3) + a_OBBDim(2) * AbsR(1,3);
rb = b_OBBDim(1) * AbsR(3,2) + b_OBBDim(2) * AbsR(3,1);
if (abs(t(2) * R(1,3) - t(1) * R(2,3)) > ra + rb); contact = 0; disp('NO CONTACT'); end

if(contact); disp('CONTACT'); end

%% Temp code
M1 = [[1,2,3];[4,5,6];[7,8,9]];
M2 = [[10,11,12];[13,14,15];[16,17,18]];
vec1 = [19;20;21];
M3 = [[22,23,24];[25,26,27];[28,29,30]];
M4 = [[31,32,33];[34,35,36];[37,38,39]];
disp('M1 * M2'); M1 * M2
disp('M1 * M2^t'); M1 * M2'
disp('M1^t * M2'); M1' * M2
disp('M1 * vec1'); M1 * vec1
disp('M1^t * vec1'); M1' * vec1
disp('M1 * M3'); M1 * M3
disp('M3 * M1'); M3 * M1
disp('M1 * M3^t'); M1 * M3'
disp('M3 * M1^t'); M3 * M1'
disp('M1^t * M3'); M1' * M3
disp('M3^t * M1'); M3' * M1
disp('M3^t * M4'); M3' * M4
disp('M3^t * vec1'); M3' * vec1
disp('M1^t * M3^t'); M1' * M3'
disp('M3^t * M1^t'); M3' * M1'



