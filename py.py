import numpy as np
import cv2
from matplotlib import pyplot as plt

img = cv2.imread('test5.png', cv2.IMREAD_UNCHANGED)
print(img.shape)
imsizex = img.shape[0]
imsizey = img.shape[1]


templ_right = [(-1,-1),(-1,0),(-1,1)]
templ_left = [(1,-1),(1,0),(1,1)]
templ_up = [(-1,1),(0,-1),(1,-1)]
templ_down = [(-1,1),(0,1),(1,1)]
templs = [templ_right, templ_left, templ_up, templ_down]

# meaning of new vector: 0R - 1G - 2B - 3A - 4 X of closest pixel - 5 Y of closest pixel - 6 DIST from closest pixel 
info = np.concatenate( (img, np.zeros((imsizex,imsizey,3))) , axis = 2 )

def dist(x1,y1,x2,y2):
#    print("x1", x1)
#    print("y1", y1)
#    print("x2", x2)
#    print("y2", y2)
    return np.sqrt( (x1-x2)**2  +  (y1-y2)**2 )



# init
for x in range(len(info)):
    for y in range(len(info[0])):
        if img[x][y][3] != 0:
            info[x][y][4] = x
            info[x][y][5] = y
            info[x][y][6] = 0
        else:
            info[x][y][4] = float('nan')
            info[x][y][5] = float('nan')
            info[x][y][6] = np.inf


# right sweep
# print("right sweep")
for x in range(imsizex):
    for y in range(imsizey):

        for k in range(3):
            cx = x + templ_right[k][0]
            cy = y + templ_right[k][1]
            if cx >= 0 and cx < imsizex and cy >= 0 and cy < imsizey:    
                newdist = dist(x,y,info[cx][cy][4],info[cx][cy][5])
                #print("newdist", newdist, "dist ref", info[x][y][6])
                if newdist < info[x][y][6]:
                    #print("got em")
                    info[x][y][0] = info[cx][cy][0]
                    info[x][y][1] = info[cx][cy][1]
                    info[x][y][2] = info[cx][cy][2]
                    info[x][y][3] = info[cx][cy][3]
                    
                    info[x][y][4] = info[cx][cy][4]
                    info[x][y][5] = info[cx][cy][5]
                    info[x][y][6] = newdist
# plt.imshow(info[:,:,:4])
# plt.show()
                    
# left sweep
# print("left sweep")
for x in range(imsizex-1, 0-1 ,-1):
    for y in range(imsizey):

        for k in range(3):
            cx = x + templ_left[k][0]
            cy = y + templ_left[k][1]
            if cx >= 0 and cx < imsizex and cy >= 0 and cy < imsizey:    
                newdist = dist(x,y,info[cx][cy][4],info[cx][cy][5])
                #print("newdist", newdist, "dist ref", info[x][y][6])
                if newdist < info[x][y][6]:
                    #print("got em")
                    info[x][y][0] = info[cx][cy][0]
                    info[x][y][1] = info[cx][cy][1]
                    info[x][y][2] = info[cx][cy][2]
                    info[x][y][3] = info[cx][cy][3]
                    
                    info[x][y][4] = info[cx][cy][4]
                    info[x][y][5] = info[cx][cy][5]
                    info[x][y][6] = newdist

# plt.imshow(info[:,:,:4])
# plt.show()

# down sweep
# print("down sweep")
for y in range(imsizey):
    for x in range(imsizex):

        for k in range(3):
            cx = x + templ_down[k][0]
            cy = y + templ_down[k][1]
            if cx >= 0 and cx < imsizex and cy >= 0 and cy < imsizey:    
                newdist = dist(x,y,info[cx][cy][4],info[cx][cy][5])
                #print("newdist", newdist, "dist ref", info[x][y][6])
                if newdist < info[x][y][6]:
                    #print("got em")
                    info[x][y][0] = info[cx][cy][0]
                    info[x][y][1] = info[cx][cy][1]
                    info[x][y][2] = info[cx][cy][2]
                    info[x][y][3] = info[cx][cy][3]
                    
                    info[x][y][4] = info[cx][cy][4]
                    info[x][y][5] = info[cx][cy][5]
                    info[x][y][6] = newdist
# plt.imshow(info[:,:,:4])
# plt.show()


# up sweep
# print("up sweep")
for y in range(imsizey-1, 0-1, -1):
    for x in range(imsizex):

        for k in range(3):
            cx = x + templ_down[k][0]
            cy = y + templ_down[k][1]
            if cx >= 0 and cx < imsizex and cy >= 0 and cy < imsizey:    
                newdist = dist(x,y,info[cx][cy][4],info[cx][cy][5])
                #print("newdist", newdist, "dist ref", info[x][y][6])
                if newdist < info[x][y][6]:
                    info[x][y][0] = info[cx][cy][0]
                    info[x][y][1] = info[cx][cy][1]
                    info[x][y][2] = info[cx][cy][2]
                    info[x][y][3] = info[cx][cy][3]
                    
                    info[x][y][4] = info[cx][cy][4]
                    info[x][y][5] = info[cx][cy][5]
                    info[x][y][6] = newdist
                    
         
                    

def diffuse(image):
    newimage = image
    for x in range(imsizex):
        for y in range(imsizey):
            truerad = int(image[x][y][6] * 0.92)
            for k in range(0,3):
                newvalue = 0
                count = 0
                if x - truerad > 0:
                    newvalue += image[x-truerad][y][k]; count=count+1;
                if x + truerad < imsizex:
                    newvalue += image[x+truerad][y][k]; count=count+1;
                if y - truerad > 0:
                    newvalue += image[x][y-truerad][k]; count=count+1;
                if y + truerad < imsizey:
                    newvalue += image[x][y+truerad][k]; count=count+1;
                newvalue = newvalue/count
                newimage[x][y][k] = np.round(newvalue)
    return newimage
                

# info = diffuse(info)
# info = diffuse(info)
# info = diffuse(info)
# info = diffuse(info)

cv2.imwrite("out.png", info[:,:,:3])
