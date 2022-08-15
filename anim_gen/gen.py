from PIL import Image
import numpy

inpath = "./tiles.png"
outpath = "./out/tiles%d.png"
im = Image.open(inpath)

streamcols = []
data = numpy.asarray(im)
streamcols.append(data[0][35])
streamcols.append(data[0][36])
streamcols.append(data[4][107])
del data

def flow(inarr, outarr, xs, ys, xe, ye, dx, dy):
	for x in range(xs, xe):
		for y in range(ys, ye):
			if x - dx < xs:
				outarr[y][x] = inarr[y-dy][x-dx+(xe-xs)]
			elif x - dx >= xe:
				outarr[y][x] = inarr[y-dy][x-dx-(xe-xs)]
			elif y - dy < ys:
				outarr[y][x] = inarr[y-dy+(ye-ys)][x-dx]
			elif y - dy >= ye:
				outarr[y][x] = inarr[y-dy-(ye-ys)][x-dx]
			else:
				outarr[y][x] = inarr[y-dy][x-dx]
def cpy(inarr, outarr, xs, ys, xe, ye, xos, yos):
	for x in range(xs, xe):
		for y in range(ys, ye):
			outarr[y-ys+yos][x-xs+xos] = inarr[y][x]
def cpy_masked(inarr, outarr, xs, ys, xe, ye, xos, yos, maskcols):
	for x in range(xs, xe):
		for y in range(ys, ye):
			for c in maskcols:
				ox, oy = x-xs+xos, y-ys+yos
				if inarr[oy][ox][0] == c[0] and inarr[oy][ox][1] == c[1] and inarr[oy][ox][2] == c[2] and inarr[oy][ox][3] == c[3]:
					outarr[oy][ox] = inarr[y][x]
def cpyw(inarr, outarr, xs, ys, w, h, xos, yos):
	for x in range(xs, xs+w):
		for y in range(ys, ys+h):
			outarr[y-ys+yos][x-xs+xos] = inarr[y][x]
xdw, ydw = 35, 0
xup, yup = 51, 0
xlf, ylf = 64, 3
xrg, yrg = 80, 3
xdwt, ydwt = 32, 0
xupt, yupt = 48, 0
xlft, ylft = 64, 0
xrgt, yrgt = 80, 0

def cpy_turn(inarr, outarr, xs, ys, mix):
	for x in range(xs, xs+16):
		for y in range(ys, ys+16):
			for c in streamcols:
				if inarr[y][x][0] == c[0] and inarr[y][x][1] == c[1] and inarr[y][x][2] == c[2] and inarr[y][x][3] == c[3]:
					px, py = x-xs, y-ys
					outarr[y][x] = mix(px, py, inarr[ydwt+py][xdwt+px], inarr[yupt+py][xupt+px], inarr[ylft+py][xlft+px], inarr[yrgt+py][xrgt+px])
def coleq(a, b):
	return a[0] == b[0] and a[1] == b[1] and a[2] == b[2] and a[3] == b[3]
def mixf(px, py, v1, v2):
	if px == py:
		return streamcols[2] if coleq(v1, streamcols[1]) or coleq(v2, streamcols[1]) else streamcols[0]
	return v1 if px>py else v2

def next_frame(im):
	data = numpy.asarray(im)
	datanew = numpy.asarray(im)
	flow(data, datanew, xdw, ydw, 45, 16, 0, 1) #d # direct
	flow(data, datanew, xup, yup, 61, 16, 0, -1) #u
	flow(data, datanew, xlf, ylf, 80, 13, -1, 0) #l
	flow(data, datanew, xrg, yrg, 96, 13, 1, 0) #r
	cpy_masked(data, datanew, 80, 3, 96, 13, 0, 115, streamcols) # broken ai
	#cpyw(data, datanew, xdw, ydw, 10, 3, 99, 0) #ur - u
	#cpyw(data, datanew, 93,  3,   3, 10, 109, 3) #ur - r
	#cpyw(data, datanew, xup, yup, 10, 3, 115, 0) #ru - u
	#cpyw(data, datanew, 77,  3,   3, 10, 125, 3) #ru - r
	cpy_turn(data, datanew, 96, 0, lambda px,py,d,u,l,r: mixf(15-px,py,d,r)) # ur
	cpy_turn(data, datanew, 112, 0, lambda px,py,d,u,l,r: mixf(15-px,py,u,l)) # ru
	cpy_turn(data, datanew, 0, 16, lambda px,py,d,u,l,r: mixf(px,py,l,d)) # rd
	cpy_turn(data, datanew, 16, 16, lambda px,py,d,u,l,r: mixf(px,py,r,u)) # dr
	cpy_turn(data, datanew, 32, 16, lambda px,py,d,u,l,r: mixf(15-px,py,l,u)) # dl
	cpy_turn(data, datanew, 48, 16, lambda px,py,d,u,l,r: mixf(15-px,py,r,d)) # ld
	cpy_turn(data, datanew, 64, 16, lambda px,py,d,u,l,r: mixf(px,py,u,r)) # lu
	cpy_turn(data, datanew, 80, 16, lambda px,py,d,u,l,r: mixf(px,py,d,l)) # ul
	return Image.fromarray(datanew)
for i in range(16):
	im = next_frame(im)
	print(f"frame {i+1} generated!")
	im.save(outpath.replace("%d", str((i+1)%16)))
	print(f"frame {i+1} saved!")
