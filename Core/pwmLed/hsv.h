typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
}RgbColor;

typedef struct  {
	unsigned char h;
	unsigned char s;
	unsigned char v;
}HsvColor;

 HsvColor RgbToHsv( RgbColor rgb);
 RgbColor HsvToRgb( HsvColor hsv);
