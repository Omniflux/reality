import time

t = time.time()
vstart = 1
vertices = []
normals = []
uvs = []
faces = []
polycount = 0

f = open("/tmp/out.obj", "w")

for a in poser.Scene().Actors():
	if (a.Visible() == 0):
		continue

	if (a.IsBodyPart() and a.ItsFigure()):
		if ((a.ItsFigure().SkinType() != poser.kSkinTypePOSER) and a.SubdivideWithFigure()):
			continue
	else:
		if (not (a.IsProp())):
			continue
	
	g = a.SubdivGeometry()
	if (g == None):
		continue

	norms = g.WorldNormals()
	if (norms == None):
		norms = g.Normals()	
	sets = g.Sets()
	polys = g.Polygons()
	
	tverts = g.TexVertices()
	tsets = g.TexSets()
	tpolys = g.TexPolygons()
	
	wverts = g.WorldVertices()
	if (wverts == None):
		wverts = g.Vertices()
		
	for v in wverts:
		vertices.append((v.X(), v.Y(), v.Z()))
	
	for n in norms:
		normals.append((v.X(), v.Y(), v.Z()))
	
	if (tverts != None):
		for tex in tverts:
			uvs.append((tex.U(), tex.V()))
	
	for i in range(g.NumPolygons()):
		p = g.Polygon(i)
		pstart = p.Start()
		face = []
		for i in range(p.NumVertices()):
			face.append(vstart + sets[i+pstart])
		faces.append(face)
		
	polycount = polycount + g.NumPolygons()
	vstart = vstart + g.NumVertices()

for fig in poser.Scene().Figures():
	if (fig.SkinType() == poser.kSkinTypePOSER):
		continue
		
	g = fig.SubdivGeometry()
	if (g == None):
		continue

	norms = g.WorldNormals()
	if (norms == None):
		norms = g.Normals()	
	sets = g.Sets()
	polys = g.Polygons()
	
	tverts = g.TexVertices()
	tsets = g.TexSets()
	tpolys = g.TexPolygons()
	
	wverts = g.WorldVertices()
	if (wverts == None):
		wverts = g.Vertices()
		
	for v in wverts:
		vertices.append((v.X(), v.Y(), v.Z()))
	
	for n in norms:
		normals.append((v.X(), v.Y(), v.Z()))
	
	if (tverts != None):
		for tex in tverts:
			uvs.append((tex.U(), tex.V()))
	
	for i in range(g.NumPolygons()):
		p = g.Polygon(i)
		pstart = p.Start()
		face = []
		for i in range(p.NumVertices()):
			face.append(vstart + sets[i+pstart])
		faces.append(face)
		
	polycount = polycount + g.NumPolygons()
	vstart = vstart + g.NumVertices()

diff = time.time() - t
print diff, "seconds for traversing geometry"
print vstart-1, "vertices"
print polycount, "polygons"

t = time.time()

for (x, y, z) in vertices:
	line = 'v ' + str(x) + ' ' + str(y) + ' ' + str(z) + '\n'
	f.write(line)
	
for (x, y, z) in normals:
	line = 'vn ' + str(x) + ' ' + str(y) + ' ' + str(z) + '\n'
	f.write(line)
	
for face in faces:
	line = 'f'
	for v in face:
		line = line + ' ' + str(v) + '//' + str(v)
	line = line + '\n'
	f.write(line)

f.close()

diff = time.time() - t
print diff, "seconds for writing OBJ file."
