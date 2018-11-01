#ifndef _AMF_H_
#define _AMF_H_
#include <iostream>
#include <fstream>
using namespace std;

#define IMAGE_NUMBER 171
#define MASK_L 3

struct bmpheader
{
	//char c1;
	//char c2;
	unsigned int FileSize;		    // file size in bytes, dword (4 bytes)
	unsigned int Reserve;		    // reserved for later use
	unsigned int BitmapDataOffset;	// Bitmap Data Offset, beginning of data
	unsigned int BitmapHeaderSize;	// Bitmap Header Size, size of bmp info
	unsigned int width;		        // horizontal width in pixels
	unsigned int height;	        // vertical height in pixels
	short int planes;		        // number of planes in bitmap
	short int bits;			        // bits per pixel
	unsigned int Compression;		// compression mode
	unsigned int BitmapDataSize;	// Bitmap Data Size, size of data
	unsigned int H_Rosolution;		// horizontal resolution, pixels per meter
	unsigned int  U_Rosolution;		// vertical resolution
	unsigned int UsedColorsSize;	// number of colors used
	unsigned int ImportantColors;   // important colors
};

typedef struct
{
	unsigned int nr;      // 影像的高度
	unsigned int nc;      // 影像的寬度
	unsigned char* m;     // 彩色影像(24bits) 
}cam_img;


unsigned char **uc2d(int nr, int nc);
unsigned char** r2d(unsigned char *m, int nr, int nc);
void d2r(unsigned char*m, unsigned char** I, int nr, int nc);
void Print_BMP_Header(bmpheader h);
void get_tmpM(unsigned char** tmp_M, unsigned char**I, int index_i, int index_j);
void conv(unsigned char** c, unsigned char** I, int M[]);
bool flag_er(unsigned char** c, int M[]);
bool flag_di(unsigned char** c, int M[]);
unsigned char** lable4(unsigned char *m, int nr, int nc);
void DoSearch(cam_img nowSrc, int C[]);
void rectangle(unsigned char* m, int C[], int nr, int nc);




void initializeList()
{
	// 寫入bmp檔案名字到txt檔案 
	ofstream fileout;
	char filep[128] = "AMF_img/";
	char name[20] = "filelist.txt";

	fileout.open(strcat(filep, name), ios::out);

	if (!fileout)
		cerr << "Can't open the file" << endl;
	for (int i = 0; i <= IMAGE_NUMBER; i++)
		fileout << i << ".bmp" << '\n';
	fileout.close();        // 要關閉 防止FILE出錯
}

cam_img uc2D_Initial(unsigned int nc, unsigned int nr)
{
	cam_img imgSrc;
	imgSrc.nc = nc;
	imgSrc.nr = nr;

	unsigned char *m;
	m = (unsigned char *)malloc(nc * nr * 3 * sizeof(unsigned char));
	imgSrc.m = m;
	
	return imgSrc;
}

unsigned char* Load_bmp_8bit(char SrcPath[], int*Nr, int*Nc, int*depth)
{
	bool ok = true;         // assume file is handlable
	int row, col, padding;
	int r, g, b;
	int i = 0, j, tmp;

	char temp;
	bmpheader header;
	ifstream file;

	file.open(SrcPath, ios::binary);   // 以二進位制模式開啟

	if (file.is_open())
	{
		ok = (bool)(file.seekg(0x0002));                           // seekg:設置輸入文件流的文件流指標位置(identifier(ID)2個Bytes)	
		ok = (bool)(file.read((char *)&header, sizeof(header)));   // 讀bmp檔案的標頭檔
	}

	*Nc = header.width;
	*Nr = header.height;
	*depth = header.bits;
	unsigned char* m = (unsigned char *)malloc((*Nc) * (*Nr) * 3 * sizeof(unsigned char));
	unsigned char* n = (unsigned char *)malloc((*Nc) * (*Nr) * sizeof(unsigned char));

	// bitmap must be 24 bit, uncompressed
	if (!ok)
	{
		cout << "File Problems.\n";
	}
	else
		if ((header.bits != 32 && header.bits != 24 && header.bits != 8) || header.Compression != 0)
		{
			ok = false;
			cout << "File Limitation: Bitmap file must be 24 bit with no compression.\n";
		}

	if (ok)
	{
		// move offset to rgb_raw_data_offset to get RGB raw data
		file.seekg(header.BitmapDataOffset);

		//calculate padding bytes in one row (COL數目要是4的倍數，因為bgrbgr....有缺的話都要補0)
		padding = ((header.bits*header.width + 31) / 32) * 4 - ((header.bits / 8)*header.width);

		// 將BMP rawData給m
		if (*depth == 24)
			for (row = 0; row < header.height; row++)
			{
				for (col = 0; col < header.width; col++)
					for (int c = 0; c < 3; c++)
					{
						file.read((char *)&temp, sizeof(temp));
						m[i] = temp;
						i++;
					}
				file.seekg(padding, ios_base::cur);
			}

		// 背景計算
		for (i = 0; i < *Nr; i++)
			for (j = 0; j<*Nc; j++)
			{
				tmp = i*(*Nc) + j;                   // bmp彩色圖像為左下到右上，依序為BGR
				b = m[tmp * 3];
				g = m[tmp * 3 + 1];
				r = m[tmp * 3 + 2];
		
				n[tmp] = (r + g + b) / 3;      // 大小為NR*NC 
			}
		//Print_BMP_Header(header);
		header.bits = 8;
		*depth = header.bits;
		return n;
	}
	
}

unsigned char* Load_bmp_24bit(char SrcPath[], int*Nr, int*Nc, int*depth)
{
	bool ok = true;         // assume file is handlable
	int row, col, padding;
	int i = 0;

	char temp;
	bmpheader header;
	ifstream file;

	file.open(SrcPath, ios::binary);   // 以二進位制模式開啟

	if (file.is_open())
	{
		ok = (bool)(file.seekg(0x0002));                           // seekg:設置輸入文件流的文件流指標位置(identifier(ID)2個Bytes)	
		ok = (bool)(file.read((char *)&header, sizeof(header)));   // 讀bmp檔案的標頭檔
	}

	*Nc = header.width;
	*Nr = header.height;
	*depth = header.bits;
	unsigned char* m = (unsigned char *)malloc((*Nc) * (*Nr) * 3 * sizeof(unsigned char));


	// bitmap must be 24 bit, uncompressed
	if (!ok)
	{
		cout << "File Problems.\n";
	}
	else
		if ((header.bits != 32 && header.bits != 24 && header.bits != 8) || header.Compression != 0)
		{
			ok = false;
			cout << "File Limitation: Bitmap file must be 24 bit with no compression.\n";
		}

	if (ok)
	{
		// move offset to rgb_raw_data_offset to get RGB raw data
		file.seekg(header.BitmapDataOffset);

		//calculate padding bytes in one row (COL數目要是4的倍數，因為bgrbgr....有缺的話都要補0)
		padding = ((header.bits*header.width + 31) / 32) * 4 - ((header.bits / 8)*header.width);

		if (*depth == 24)
			for (row = 0; row < header.height; row++)
			{
				for (col = 0; col < header.width; col++)
					for (int c = 0; c < 3; c++)
					{
						file.read((char *)&temp, sizeof(temp));
						m[i] = temp;
						i++;
					}	
				file.seekg(padding, ios_base::cur);
			}
	}
	//Print_BMP_Header(header);
	return m;
} // end Load_bmp_24bit()

void erosion(cam_img nowSrc)
{
	int mask[9] = { 0,1,0, 1,1,1, 0,1,0 };   // 遮罩做conv用 (十字架的會變0)
	unsigned char**tmpM;                     // ima的3*3，和mask做conv
	int tag = (MASK_L - 1) / 2;
	int nr = nowSrc.nr;
	int nc = nowSrc.nc;
	int M = nr + 2*tag;
	int N = nc + 2*tag;
	int row = 0, col = 0;
	unsigned char ** ero;
	unsigned char ** ima;
	unsigned char ** c = uc2d(MASK_L, MASK_L);   // 遮罩運算結果
	bool flag;

	ima = uc2d(M, N);                  // 分配記憶體(二維)，做Convolution用
	tmpM = uc2d(MASK_L, MASK_L);       // 遮罩運算結果
	ero = r2d(nowSrc.m, nr, nc);       // 將一維(m為raw data)轉成二維

	// padding ， ero的值給ima     (0,0)~(nr,nc) -> (1,1)~(M-tag, N-tag)
	for (int i = tag; i < M - tag; i++)
	{
		for (int j = tag; j < N - tag; j++)
		{
			ima[i][j] = ero[row][col];
			col += 1;
		}
		row += 1;
		col = 0;
	}

	// conv 
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
		{
			get_tmpM(tmpM, ima, i, j);       // 取出ima的3X3矩陣
			conv(c, tmpM, mask);             // 遮罩運算
			flag = flag_er(c, mask);		 // 判斷遮罩和mask

			// erode
			if (flag)
				ero[i][j] = 0;
		}
	d2r(nowSrc.m, ero, nr, nc);             // ero(二維)轉成nowSrc.m(一維)
} // end erosion

void dilation(cam_img nowSrc)
{
	int mask[9] = { 0,1,0, 1,1,1, 0,1,0 };   // 遮罩做conv用 (十字架的會變0)
	unsigned char**tmpM;                     // ima的3*3，和mask做conv
	int tag = (MASK_L - 1) / 2;
	int nr = nowSrc.nr;
	int nc = nowSrc.nc;
	int M = nr + 2*tag;
	int N = nc + 2*tag;
	int row = 0, col = 0;
	unsigned char ** dil;
	unsigned char ** ima;
	unsigned char ** c = uc2d(MASK_L, MASK_L);  // 遮罩運算結果
	bool flag;

	ima = uc2d(M, N);                  // 分配記憶體(二維)，做Convolution用
	tmpM = uc2d(MASK_L, MASK_L);       // 遮罩運算結果
	dil = r2d(nowSrc.m, nr, nc);       // 將一維(m為raw data)轉成二維

	// padding ， dil的值給ima     (0,0)~(nr,nc) -> (1,1)~(M-tag, N-tag)
	for (int i = tag; i < M - tag; i++)
	{
		for (int j = tag; j < N - tag; j++)
		{
			ima[i][j] = dil[row][col];
			col += 1;
		}
		row += 1;
		col = 0;
	}

	// conv 
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
		{
			get_tmpM(tmpM, ima, i, j);      // 取出ima的3X3矩陣
			conv(c, tmpM, mask);            // 遮罩運算
			flag = flag_di(c, mask);		// 判斷遮罩和mask

			// dilate
			if (flag)
				dil[i][j] = 255;
		}
	d2r(nowSrc.m, dil, nr, nc);
} // end dilation()


void get_tmpM(unsigned char** tmp_M, unsigned char**I, int index_i, int index_j)
{
	int row = 0, col = 0;
	for (int i = index_i; i < index_i + MASK_L; i++)
	{
		for (int j = index_j; j < index_j + MASK_L; j++)
		{
			tmp_M[row][col] = I[i][j];
			col++;
		}
		row++;
		col = 0;
	}
}


void conv(unsigned char**c, unsigned char** I, int M[])
{
	for (int i = 0; i < MASK_L; i++)
		for (int j = 0; j < MASK_L; j++)
			c[i][j] = I[i][j] * M[i*MASK_L + j];         
}


bool flag_er(unsigned char** c, int M[])
{
	for (int i = 0; i < MASK_L; i++)
		for (int j = 0; j < MASK_L; j++)
			if (int(c[i][j]) == 0 && M[i*MASK_L + j] == 1)     // MASK為1但c為0就侵蝕	
				return true;
	return false;
}


bool flag_di(unsigned char** c, int M[])
{
	for (int i = 0; i < MASK_L; i++)
		for (int j = 0; j < MASK_L; j++)
			if (int(c[i][j]) == 255)   // c在MASK為1的地方有值就膨脹	
				return true;
	return false;
}


// 將BITMAP串流資料變成二維陣列(raw data to 二維陣列)
unsigned char** r2d(unsigned char *m, int nr, int nc)
{
	int tmp = 0;
	unsigned char** I = uc2d(nr, nc);
	for (int row = nr - 1; row >= 0; row--)
		for (int col = 0; col < nc; col++)
		{
			I[row][col] = m[tmp];
			tmp++;
		}
	return I;
}


// 將二列陣列轉維raw data
void d2r(unsigned char*m, unsigned char** I, int nr, int nc)
{
	int tmp = 0;
	for (int row = nr - 1; row >= 0; row--)
		for (int col = 0; col < nc; col++)
		{
			m[tmp] = int(I[row][col]);
			tmp++;
		}
}


unsigned char **uc2d(int nr, int nc)
{
	if (nr <= 0 || nc <= 0) return NULL;

	int i;
	unsigned char **Array, *pData;
	Array = (unsigned char**)calloc(nr, sizeof(unsigned char*));   // 宣告nr個(unsigned char*)大小的空間

	if (Array == NULL)
	{
		printf("uc2d_Initialize: Fail to allocate memory\n");
		return NULL;
	}
	pData = (unsigned char*)calloc(nr*nc, sizeof(unsigned char));

	for (i = 0; i < nr; i++)
	{
		Array[i] = pData;
		pData += nc;            // 初始地址
	}

	return Array;            // 分配nr塊記憶體，每個nr記憶體存放nc大小記憶體的記憶體位置
}// end **uc2d()


// 找到人物的中心點C(x1, x2)在draw出來
void DoSearch(cam_img nowSrc, int C[])
{
	int LB = 0, lb;                   // 四連通標記
	int tmp = 0;
	int flag = 0;                     // 出現標籤次數的中心值
	int nr = nowSrc.nr;
	int nc = nowSrc.nc;
	
	int threshhold = 50;              // 前景面積pixel點
	int x1, x2, n;
	x1 = x2 = n = 0;

	unsigned char** I;               // 標記的圖
	I = lable4(nowSrc.m, nr, nc);

	 // 找到標籤最大的LB
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
			if (I[i][j] > LB)
				LB = I[i][j];

	int *tmp_C = (int*)calloc(LB, sizeof(int));            // 存放每個區塊的x, y
	int *tag = (int*)calloc(LB, sizeof(int));              // 每個標記的值


	// 找到每個標籤值有的數量
	for (int lb = 1; lb < LB; lb++)
	{
		for (int i = 0; i < nr; i++)
			for (int j = 0; j < nc; j++)
				if (int(I[i][j]) == lb)
					tag[tmp] += 1;
		tmp++;
	}


	tmp = 0;
	// 掃描tag的每個標籤值，當點數超過閥值就儲存座標
	for (int k= 0; k < LB; k++)
	{
		if (tag[k] > threshhold)                // 標記超過50個點才有影響
		{
			flag = tag[k] / 2 + 1;

			// 從I矩陣中找到C(x1,x2) -> 亮度範圍的中心點座標
			for (int i = 0; i < nr; i++)
				for (int j = 0; j < nc; j++)
					if (int(I[i][j]) == (k + 1))
					{
						n++;
						if (flag == n)
						{
							tmp_C[tmp * 2] = i;
							tmp_C[tmp * 2 + 1] = j;
							tmp++;

						}
					}
		}
	}

	// 計算每個面積的C(x1,x2)取平均值
	if (tmp > 0)
	{
		for (int i = 0; i < tmp; i++)
		{
			x1 = x1 + tmp_C[2 * i];
			x2 = x2 + tmp_C[2 * i + 1];
		}
		if (x1 != 0 && x2 != 0)
		{
			C[0] = x1 / tmp;
			C[1] = x2 / tmp;
		}
	}
	free(tmp_C);
	free(tag);
}// end DoSearch()


unsigned char** lable4(unsigned char *m, int nr, int nc)
{
	int mask[9] = { 0,1,0, 1,1,0, 0,0,0 };
	int M = nr + 2;
	int N = nc + 2;
	int row = 0, col = 0;
	int LB = 1;       // 四連通標記
	int r, t, p;

	unsigned char** I = uc2d(nr, nc);
	unsigned char** ima;
	unsigned char** tmpM;
	unsigned char** c = uc2d(MASK_L, MASK_L);          // 遮罩運算結果
	
	ima = uc2d(M, N);                // 分配記憶體(二維)，做Convolution用
	tmpM = uc2d(MASK_L, MASK_L);     // 取出ima的3X3遮罩
	I = r2d(m, nr, nc);              // 將一維(m為raw data)轉成二維

	// padding ， I的值給ima     (0,0)~(nr,nc) -> (1,1)~(M-tag, N-tag)
	for (int i = 1; i < M - 1; i++)
	{
		for (int j = 1; j < N - 1; j++)
		{
			ima[i][j] = I[row][col];
			col += 1;
		}
		row += 1;
		col = 0;
	}
	
	// pass 1:scanning
	for (int i = 0; i < nr; i++)
		for (int j = 0; j < nc; j++)
		{
			get_tmpM(tmpM, ima, i, j);        // 取出ima的3X3矩陣
			conv(c, tmpM, mask);              // 遮罩運算

		    // 四連通標記法
			r = int(c[0][1]) / 255;
			t = int(c[1][0]) / 255;
			p = int(c[1][1]) / 255;
			if (p == 0) continue;
			else
			{
				if (r == 0 && t == 0)
				{
					I[i][j] = LB;                      // set lp a new label
					LB++;
				}
				else if (r == 1 && t == 0)            // lp=lr
					I[i][j] = I[i - 1][j];
				else if (r == 0 && t == 1)            // lp=lt
					I[i][j] = I[i][j - 1];
				else if (r == 1 && t == 1)
				{
					if (I[i - 1][j] == I[i][j - 1])    // lp=lr v lt
						I[i][j] = I[i - 1][j];
					else                               // lr != lt
					{
						I[i][j] = I[i - 1][j];         // lp=lr
					}
				}
			} // end else
		}// end for

	// pass 2:merging classes(lt=lr)
	for (int i = nr - 1; i >= 0; i--)
		for (int j = nc - 1; j >= 0; j--)
			if (I[i][j] < I[i][j - 1] && I[i][j]>0)
				I[i][j - 1] = I[i][j];
	return I;
} // end lable4()


void drawImage(cam_img drawSrc, int C[])
{
	int nr, nc;
	
	nr = drawSrc.nr;
	nc = drawSrc.nc;
	unsigned char* m = drawSrc.m;

	rectangle(m, C, nr, nc);

	drawSrc.m = m;
} // end drawImage()


  /* 框出(x1-20, x2-50), (x1+20, x2-50)
         (x1-20, x2+50), (x1+20, x2+50) 的長方形   */
void rectangle(unsigned char* m, int C[], int nr, int nc)
{
	int tmp;
	int row = 0;      // 查看row的位置
	int r, g, b;
	int r_weight = 42, r_height = 80;
	int x1 = C[0], x2 = C[1];
	int position_x[2] = { x1 - r_height / 6, x1 + r_height};          // 長方形的x座標(影響高度)
	int position_y[2] = { x2 - r_weight / 2, x2 + r_weight / 2 };     // 長方形的y座標(影響寬度)

	if (x1 != 0 && x2 != 0)
	{
		for (int i = nr - 1; i >= 0; i--)
		{
			for (int j = 0; j < nc; j++)
			{
				tmp = i*nc + j;       // 左上到右下依序為RGB

				// 框出紅色長方形
				if (row == position_x[0] || row == position_x[1])      // 長方形上下
				{
					if (position_y[0] <= j && j < position_y[1])
					{
						m[tmp * 3] = 0;             // b
						m[tmp * 3 + 1] = 0;         // g
						m[tmp * 3 + 2] = 255;       // r
					}
				}
				else if (j == position_y[0] || j == position_y[1])     // 長方形左右
					if (position_x[0] <= row && row < position_x[1])
					{
						m[tmp * 3] = 0;             // b
						m[tmp * 3 + 1] = 0;         // g
						m[tmp * 3 + 2] = 255;       // r
					}
			}
			row++;
		}
	}
	C[0] = C[1] = 0;
} // end rectangle

void Save_bmp_8bit(char path[], unsigned char* m, int Nr, int Nc)
{
	bool ok = true;         // for checking file output problems
	char i = 'B', d = 'M';
	int row, col, padding;
	int j = 0;
	char temp;
	bmpheader header;
	int byte_per_pixel = 1;
	unsigned char Palette[256 * 4];
	int nbpal;
	ofstream file;

	file.open(path, ios::binary);

	ok = file.is_open();
	if (!ok)
	{
		cout << "File Problems.\n";
	}

	//raw data offset
	header.BitmapDataOffset = 1074;
	header.BitmapHeaderSize = 40;
	header.height = Nr;
	header.width = Nc;
	header.planes = 1;
	header.bits = 8;
	header.Compression = 0;
	header.BitmapDataSize = Nr*Nc;
	header.H_Rosolution = 0;
	header.U_Rosolution = 0;
	header.UsedColorsSize = 0;
	header.ImportantColors = 0;

	// file size
	header.FileSize = Nr * Nc * byte_per_pixel + header.BitmapDataOffset;
	header.Reserve = 0;
	// BM id header
	ok = (bool)(file.write((char *)&i, sizeof(i)));
	ok = (bool)(file.write((char *)&d, sizeof(d)));

	// rest of header
	ok = (bool)(file.write((char *)&header, sizeof(header)));

	//Palette
	for (nbpal = 0; nbpal<256; nbpal++)
	{
		Palette[4 * nbpal + 0] = nbpal;
		Palette[4 * nbpal + 1] = nbpal;
		Palette[4 * nbpal + 2] = nbpal;
		Palette[4 * nbpal + 3] = 0;
	}

	ok = bool(file.write((char *)&Palette, sizeof(Palette)));

	// write image
	if (ok)
	{
		//calculate padding bytes in one row
		padding = ((header.bits*header.width + 31) / 32) * 4 - ((header.bits / 8)*header.width);

		// data stored bottom up, left to right
		for (row = header.height - 1; row >= 0; row--)
		{
			for (col = 0; col<header.width; col++)
			{
				temp = m[j];
				file.write((char *)&temp, sizeof(temp));
				j++;
			}
			//write padding bytes
			for (int k = 0; k < padding; ++k)
				file.put('\0');
		}
	}
} // Save_bmp_8bit

void Save_bmp_24bit(char path[], unsigned char* m, int Nr, int Nc)
{
	bool ok = true;         // for checking file output problems
	char i = 'B', d = 'M';
	int row, col, padding;
	int j = 0, tmp = 0;
	unsigned char temp, b, g, r;
	bmpheader header;
	int byte_per_pixel = 3;
	unsigned char Palette[256 * 4];
	int nbpal;
	ofstream file;

	file.open(path, ios::binary);

	ok = file.is_open();
	if (!ok)
	{
		cout << "File Problems.\n";
	}

	//raw data offset
	header.BitmapDataOffset = 54;   // 1074
	header.BitmapHeaderSize = 40;
	header.height = Nr;
	header.width = Nc;
	header.planes = 1;
	header.bits = 24;
	header.Compression = 0;
	header.BitmapDataSize = Nr*Nc * byte_per_pixel;
	header.H_Rosolution = 0;
	header.U_Rosolution = 0;
	header.UsedColorsSize = 0;
	header.ImportantColors = 0;

	// file size
	header.FileSize = Nr * Nc * byte_per_pixel + header.BitmapDataOffset;
	header.Reserve = 0;

	// BM id header
	ok = (bool)(file.write((char *)&i, sizeof(i)));
	ok = (bool)(file.write((char *)&d, sizeof(d)));

	// rest of header
	ok = (bool)(file.write((char *)&header, sizeof(header)));

	//Palette
	/*for (nbpal = 0; nbpal < 256; nbpal++)
	{
		Palette[4 * nbpal + 0] = nbpal;
		Palette[4 * nbpal + 1] = nbpal;
		Palette[4 * nbpal + 2] = nbpal;
		Palette[4 * nbpal + 3] = 0;
	}*/

	//ok = bool(file.write((char *)&Palette, sizeof(Palette)));

	// write image
	if (ok)
	{
		//calculate padding bytes in one row
		padding = ((header.bits*header.width + 31) / 32) * 4 - ((header.bits / 8)*header.width);

		// data stored bottom up, left to right
		for (row = 0; row < header.height; row++)
		{
			for (col = 0; col<header.width; col++)
			{
				tmp = row*Nc + col;
				b = m[tmp * 3];
				g = m[tmp * 3 + 1];
				r = m[tmp * 3 + 2];
				file.write((char *)&b, sizeof(b));
				file.write((char *)&g, sizeof(g));
				file.write((char *)&r, sizeof(r));
			}
			//write padding bytes
			for (int k = 0; k < padding; ++k)
				file.put('\0');
		}
	}
	file.close();
}// Save_bmp_24bit()

void Print_BMP_Header(bmpheader h)
{
	cout << endl;
	//cout << "Char 1             = " << h.c1 << endl;
	//cout << "Char 2             = " << h.c2 << endl;
	cout << "File Size          = " << h.FileSize << endl;
	cout << "Width              = " << h.width << endl;
	cout << "Height             = " << h.height << endl;
	cout << "Bits per Pixel     = " << h.bits << endl;
	cout << "Compression        = " << h.Compression << endl;
	cout << "BitmapDataOffset   = " << h.BitmapDataOffset << endl;
	cout << "BitmapDataSize     = " << h.BitmapDataSize << endl;
}

#endif