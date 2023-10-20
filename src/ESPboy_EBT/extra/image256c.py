import array
import os



class Image:

	data=array.array('B')
	size=0
	wdt=0
	hgt=0
	pixel_offset=0
	palette_offset=0
	flip=False

	
	def read_word(self,off):
		return self.data[off]+(self.data[off+1]<<8)
		
		
	def read_dword(self,off):
		return self.read_word(off)+(self.read_word(off+2)<<16)
		
			
	def write_dword(self,off,value):
		self.data[off+0]= value     &255
		self.data[off+1]=(value>>8 )&255
		self.data[off+2]=(value>>16)&255
		self.data[off+3]=(value>>24)&255
		
		
	def offset(self,x,y):

		if x<0 or y<0 or x>=self.wdt or y>=self.hgt:
			return -1;
		
		if self.flip:
			y=self.hgt-1-y
			
		return self.pixel_offset+y*self.wdt+x
		
		
	def new(self,source,w,h):
	
		self.wdt=w
		self.hgt=h
		self.pixel_offset=source.pixel_offset
		self.flip=True
		self.size=source.pixel_offset+w*h

		self.data=array.array('B',[0]*self.size)
		
		for i in range(0, source.pixel_offset):
			self.data[i]=source.data[i]
			
		for i in range(0, w*h):
			self.data[source.pixel_offset+i]=0

		return


	def load(self,name):

		self.size=os.path.getsize(name)

		file=open(name,'rb')
		self.data.fromfile(file,self.size)
		file.close()

		if self.data[0]!=ord('B') or self.data[1]!=ord('M'):
			raise ValueError('Not a BMP file!')
			
		if self.read_word(28)!=8:
			raise ValueError('Only 256-color images supported!')
			
		if self.read_dword(30)!=0:
			raise ValueError('Only uncompressed images supported!')
			
		self.pixel_offset=self.read_dword(10)
		self.palette_offset=54
		
		self.wdt=self.read_dword(18)
		self.hgt=self.read_dword(22)
		
		if self.hgt<0:
			self.flip=False
			self.hgt=-hgt
		else:
			self.flip=True

		return

		
	def save(self,name):

		self.write_dword(18,self.wdt)
		self.write_dword(22,self.hgt)
		
		file=open(name,'wb')
		self.data.tofile(file)
		file.close()

		return

		
	def get(self,x,y):
	
		ptr=self.offset(x,y)
		
		if ptr<0:
			return 0
		
		return (self.data[int(ptr)]&0xff)

		
	def set(self,x,y,color):

		ptr=self.offset(x,y)
		
		if ptr<0:
			return
		
		self.data[ptr]=(color&0xff)
			
		return


	def get_pal(self,id):
	
		return self.read_dword(self.palette_offset+id*4)