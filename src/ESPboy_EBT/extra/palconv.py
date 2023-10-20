from image256c import *
import sys



picture_src=Image()
picture_src.load(sys.argv[1])

name=os.path.splitext(sys.argv[1])[0]

print('const uint8_t '+name+'[16*3]={')

for i in range(0,16):

	col=picture_src.get_pal(i)
	
	r=col&0xff
	g=(col>>8)&0xff
	b=(col>>16)&0xff
	
	str='0x%2.2x,0x%2.2x,0x%2.2x,' % (r,g,b)

	print(str)
	
print('};')
