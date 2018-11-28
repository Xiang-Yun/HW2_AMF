/*
��l���
============== Bitmap File Header ==============
Identifier : 19778
File Size : 230454
Reserve : 0
Bitmap Data Offset : 54

============== Bitmap Info Header ==============
Bitmap Header Size : 40 Bytes
Width : 320
Height : 240
Planes : 1
Bits Per Pixel : 24
Compression : 0
Bitmap Data Size : 230400 Bytes
Color Table Size(Palette)�G0 Bytes
H-Rosolution : 0
U-Rosolution : 0
Used Colors Size : 0
Important Colors: 0
*/

#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include <direct.h>         // �Ыظ�Ƨ����禡�w
#include "my_AMF.h"

#define NC 320
#define NR 240

unsigned char **ima;
int r, g, b;
int Nr, Nc;
int depth;                   // BMP�v��bits��

int tmp = 0;
int threshold = 30;
int  fileIndex = 0;         // 172�icamImage
int C[2] = { NULL };        // ���餤���I�y��


// �v�����e(�Ĥ@�i�v��) 
cam_img camSrc = uc2D_Initial(NC, NR);

// ��l��
cam_img nowSrc = uc2D_Initial(NC, NR);
cam_img bgSrc = uc2D_Initial(NC, NR);
cam_img drawSrc = uc2D_Initial(NC, NR);


// �Ϥ����|
char filename[128];
char SrcPath[128] = "AMF_img/";
char result_position[128] = "AMF_result/";
char fgPath[128];
char bgPath[128];
char drawPath[128];

// Grafcet
void action();
void grafcet();
int X0, X1, X2, X3, X4, X5, X6, X7;


int main(int argc, char *argv[])
{
	int flag = fileIndex;
	X0 = X1 = X2 = X3 = X4 = X5 = X6 = X7 = 0;

	// ��l�ƪ���A����txt�ɮ�
	initializeList();

	// create dir for
	_mkdir("AMF_result");
	_mkdir("AMF_result/fg_img");
	_mkdir("AMF_result/bg_img");
	_mkdir("AMF_result/draw_img");


	ifstream fin("AMF_img/filelist.txt");


	X0 = 1;  // �t�αҰ�
	while (fin >> filename)
	{
		
		SrcPath[8] = '\0';                     // ��l�Ƹ��|
		result_position[11] = '\0';
		
		strcat(SrcPath, filename);            // AMF_img/filename

		// copypath
		strcpy(fgPath, result_position);
		strcpy(bgPath, result_position);
		strcpy(drawPath, result_position);

		// add filename to path
		strcat(fgPath, "fg_img/");
		strcat(fgPath, filename);
		strcat(bgPath, "bg_img/");
		strcat(bgPath, filename);
		strcat(drawPath, "draw_img/");
		strcat(drawPath, filename);


		flag = fileIndex;
		while (flag == fileIndex)
		{
			grafcet();
			cout << "X1=" << X1 << ", X2=" << X2 << ", X3=" << X3 << ", X4=" << X4 << ", X5=" << X5 << ", X6=" << X6 << ", X7=" << X7 << endl;
		}
		
		cout << "---------------------------------------------------" << endl;

	} // end while
	fin.close();
	system("pause");
	return 0;
}// end main()



void action()
{   
	if (X1 == 1)
	{
		cout << "fileIndex=" << fileIndex << endl;
		if(fileIndex==0)
			camSrc.m = Load_bmp_24bit(SrcPath, &Nr, &Nc, &depth);

		else
		{
			drawSrc.m = Load_bmp_24bit(SrcPath, &Nr, &Nc, &depth);    // �m��ϼХܵ��G
			drawSrc.nr = Nr;
			drawSrc.nc = Nc;

			nowSrc.m = Load_bmp_8bit(SrcPath, &Nr, &Nc, &depth);
		}
	}


	//�Ĥ@�i�I����l��
	else if (X2 == 1)
	{
		float R[NR][NC];
		float G[NR][NC];

		for (int i = 0; i < Nr; i++)
			for (int j = 0; j<Nc; j++)
			{
				tmp = i*Nc + j;                   // bmp�m��Ϲ������U��k�W�A�̧Ǭ�BGR
				b = camSrc.m[tmp * 3];
				g = camSrc.m[tmp * 3 + 1];
				r = camSrc.m[tmp * 3 + 2];

				if (r + g + b != 0) {
					R[i][j] = (float)r / (b + g + r);
					G[i][j] = (float)g / (b + g + r);
				}
				else {
					R[i][j] = 0;
					G[i][j] = 0;
				}
				// char*����(ASCII�N���Ʀr�A�Ҧp'T' = 84
				bgSrc.m[tmp] = nowSrc.m[tmp] = (r + g + b) / 3;      // �j�p��NR*NC(�Ƕ�)
			}
		fileIndex++;
	}

	//�ĤG�i����i��AMF��������o�i�k
	// �P�_��e�v���������Ibw(x,y)�P�I���v�������������Ibg(x, y)��̪��j�p
	else if (X3 == 1)
	{
		for (int i = 0; i < Nr; i++)
			for (int j = 0; j < Nc; j++)
			{
				tmp = i*Nc + j;
				if (nowSrc.m[tmp] > bgSrc.m[tmp])
					bgSrc.m[tmp]++;
				else if (nowSrc.m[tmp] < bgSrc.m[tmp])
					bgSrc.m[tmp]--;
			}
	}

	// �p���e�v���������Ibw(x,y)�P�I���v�������������Ibg(x, y)��̪��t�ȡA�Y |bw(x, y) - bg(x, y)|
	else if (X4 == 1)
	{
		for (int i = 0; i < Nr; i++)
			for (int j = 0; j < Nc; j++)
			{
				tmp = i*Nc + j;
				if (abs(nowSrc.m[tmp] - bgSrc.m[tmp]) > threshold)
					nowSrc.m[tmp] = 255;
				else
					nowSrc.m[tmp] = 0;
			}
	}

	// �v���B�z
	else if (X5 == 1)
	{
		erosion(nowSrc);    //�I�k
		erosion(nowSrc);
		dilation(nowSrc);   //����
		dilation(nowSrc);
	}

	// �إX����
	else if (X6 == 1)
	{
		DoSearch(nowSrc, C);       // ���إX���I
		drawImage(drawSrc, C);     // �إX��l�v��������
	}

	// �x�s�v��
	else if (X7 == 1)
	{
		Save_bmp_8bit(bgPath, bgSrc.m, Nr, Nc);
		Save_bmp_8bit(fgPath, nowSrc.m, Nr, Nc);
		Save_bmp_24bit(drawPath, drawSrc.m, Nr, Nc);
		fileIndex++;
	}

}// end action()



void grafcet()
{
	if (X0 == 1)
	{
		X0 = 0;
		X1 = 1;
	}

	else if (X1 == 1)
	{
		X1 = 0;
		if (fileIndex == 0)
			X2 = 1;
		else
			X3 = 1;
	}

	else if (X2 == 1)
	{
		X2 = 0;
		X1 = 1;
	}

	else if (X3 == 1)
	{
		X3 = 0;
		X4 = 1;
	}

	else if (X4 == 1)
	{
		X4 = 0;
		X5 = 1;
	}

	else if (X5 == 1)
	{
		X5 = 0;
		X6 = 1;
	}

	else if (X6 == 1)
	{
		X6 = 0;
		X7 = 1;
	}

	else if (X7 == 1)
	{
		X7 = 0;
		X1 = 1;
	}

	action();
}// end grafcet()