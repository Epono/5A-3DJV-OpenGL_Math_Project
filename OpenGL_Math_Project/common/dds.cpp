
#include "Common.h"
#include "dds.h"

uint32_t LoadImageDDS(uint8_t **output, uint32_t &width, uint32_t &height, const char* filename)
{
	uint32_t imageSize = 0;

	FILE* fileHandle = fopen(filename, "rb");
	if (fileHandle == nullptr)
		return 0;

	DDSFormat headerDDS;
	fread(&headerDDS, sizeof DDSFormat, 1, fileHandle);

	height = headerDDS.header.height;
	width = headerDDS.header.width;
	if (headerDDS.header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '1'))
	{
		imageSize = ((width + 3)/4) * ((height+3)/4) * sizeof(uint64_t);
	}
	else if (headerDDS.header.ddspf.fourCC == MAKEFOURCC('D', 'X', 'T', '5'))
	{
		imageSize = ((width + 3)/4) * ((height+3)/4) * sizeof(uint64_t) * 2;
	}

	*output = new uint8_t[imageSize];

	fread(*output, imageSize, 1, fileHandle);

	fclose(fileHandle);

	return imageSize;
}

void FreeImageDDS(uint8_t **output)
{
	delete[] *output;
	*output = nullptr;
}
