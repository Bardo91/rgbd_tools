/////////////////////////////////////////////////////////////////////////////////////////
//	BOVIL: core
//
//		Author: Pablo Ramon Soria
//		Date:	2014-05-03
//
/////////////////////////////////////////////////////////////////////////////////////////
// Color Cluster Space
//		Here is defined the limits of every color and functions to truncate colors.

#include <rgbd_tools/segmentation/color_clustering/types/ColorSpaceHSV8.h>

#include <cstring>


namespace rgbd {
	const c3u colorsHSV8[8] = {
	/*Black*/	c3u(0, 0, 0),
	/*White*/	c3u( 0, 0, 255 ),
	/*Blue*/	c3u( 120, 255, 255 ),
	/*Purple*/	c3u( 150, 255, 255 ),
	/*Red*/		c3u( 0, 255, 255 ),
	/*Orange*/	c3u( 15, 255, 255 ),
	/*Yellow*/	c3u( 30, 255, 255 ),
	/*Green*/	c3u( 60, 255, 255 ) };
		

	const std::string HClassStr8[36] = { "00010011", "00010011", "00100011",
			"00100011", "00100011", "01000011", "01000011", "10000011", "10000011",
			"10000011", "10000011", "10000011", "10000011", "10000011", "10000011",
			"10000011", "00000111", "00000111", "00000111", "00000111", "00000111",
			"00000111", "00000111", "00000111", "00000111", "00000111", "00000111",
			"00000111", "00001011", "00001011", "00001011", "00001011", "00001011",
			"00001011", "00010011", "00010011" };
	const std::string SClassStr8[36] = { "00000011", "00000011", "00000011",
			"00000011", "00000011", "00000011", "00000011", "00000011", "00000011",
			"11111101", "11111101", "11111101", "11111101", "11111101", "11111101",
			"11111101", "11111101", "11111101", "11111101", "11111101", "11111101",
			"11111101", "11111101", "11111101", "11111101", "11111101", "11111101",
			"11111101", "11111101", "11111101", "11111101", "11111101", "11111101",
			"11111101", "11111101", "11111101" };
	const std::string VClassStr8[36] = { "00000001", "00000001", "00000001",
			"00000001", "00000001", "00000001", "00000001", "00000001", "00000001",
			"00000001", "00000001", "00000001", "00000001", "00000001", "00000001",
			"11111110", "11111110", "11111110", "11111110", "11111110", "11111110",
			"11111110", "11111110", "11111110", "11111110", "11111110", "11111110",
			"11111110", "11111110", "11111110", "11111110", "11111110", "11111110",
			"11111110", "11111110", "11111110" };


	int bin2dec(std::string bin) {
		const char *cstr = bin.c_str();
		int len, dec = 0, i, exp;

		len = strlen(cstr);
		exp = len - 1;

		for (i = 0; i < len; i++, exp--){
			dec += cstr[i] == '1' ? int(pow(2, exp)) : 0;
		}

		return dec;
	}


	ColorClusterSpace *CreateHSVCS_8c(unsigned char MaskH, unsigned char MaskS, unsigned char MaskV) {

		unsigned char HClass[36];
		unsigned char SClass[36];
		unsigned char VClass[36];

		for (int i = 0; i < 36; i++) {
			HClass[i] = bin2dec(HClassStr8[i]) & MaskH;
			SClass[i] = bin2dec(SClassStr8[i]) & MaskS;
			VClass[i] = bin2dec(VClassStr8[i]) & MaskV;
		}

		return new ColorClusterSpace(36, HClass, SClass, VClass, colorsHSV8);

	}
} // namespace BOViL
