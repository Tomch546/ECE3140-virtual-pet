# Script to display images in same folder
# need to follow naming conventions for images to get this to work
# due to concatenation functions based on variables
# written by tc575, az292
# images by tc575
# soundfiles from @Miyolophone 

import serial
import sys, pygame
import time

# initialization - serial port name varys with device, check own and replace
serialPort = serial.Serial('/dev/tty.usbmodemSDA52F62E5A1', 115200) # port name, baud rate
serialString = "" # Used to hold data coming over UART

pygame.init() # call before other setup
pygame.display.set_caption('Virtual Pet ??') # window title
size = width, height = 250, 175 # window size
screen = pygame.display.set_mode(size) # display the screen
black = 0, 0, 0 # variables to wipe screen
pygame.mixer.init() # initialize sound

# soundbites 
hatchsound = pygame.mixer.Sound("tommygotchi_hatch.wav")
idlesound = pygame.mixer.Sound("tommygotchi_idle.wav")
eatsound = pygame.mixer.Sound("tommygotchi_eat.wav")
playsound = pygame.mixer.Sound("tommygotchi_play.wav")
bouncesound = pygame.mixer.Sound("tommygotchi_throw.wav") # replace

# display variables
background = "2"; # 1 for morning, 2 for day, 3 for night
state = 0; # mirrors states in C code, excluding non-animation states
numberOfFrames = 0; # get number of frames
frame = 0; # which frame to play
prevstate = 0; # duplicate state flag
sound = hatchsound; # set default sound

# find number of frames for a state
def findFrame(): 
    global numberOfFrames
    global state
    global background

    if (state == 1):
        numberOfFrames = 4
    elif (state == 3):
        numberOfFrames = 9
    elif (state == 4):
        numberOfFrames = 12
    elif (state == 5):
        numberOfFrames = 8
    elif (state == 6):
        numberOfFrames = 6
    elif (state == 13):
        numberOfFrames = 9
    elif (state == 14):
        numberOfFrames = 6
    elif (state == 15):
        numberOfFrames = 12
    elif (state == 16):
        numberOfFrames = 9
    else:
        numberOfFrames = 0
    print("number of frames: " + str(numberOfFrames))

# get correct sound file
def findSound(): 
    global numberOfFrames
    global state
    global background
    global sound 

    if (state == 1):
        sound = hatchsound
    elif (state == 3):
        sound = idlesound
    elif (state == 4):
        sound = eatsound
    elif (state == 5):
        sound = playsound
    elif (state == 6):
        sound = bouncesound
    elif (state == 13):
        sound = idlesound
    elif (state == 14):
        sound = bouncesound
    elif (state == 15):
        sound = eatsound
    elif (state == 16):
        sound = playsound
    else:
        sound = idlesound
    print("soundfile: " + str(sound))

# function to change display variables based on serial input
def updateVariables():
    global state;
    global background

    serialString = serialPort.readline()
    update = str(serialString.decode('Ascii'))
    print("update: " + update)

    if (update[0] == "s"):
        state = int(update[5:])
        print("state is " + str(state))
    elif (update[0] == "b"):
        background = int(update[1:])
        print(background)

# display each frame
def playAnimation():
    print("starting to display")
    global state
    global numberOfFrames
    global prevstate
    global background
    global sound
    frame = 0;
    if (state != prevstate):

        pygame.mixer.Sound.play(sound) # play audio clip

        for x in range(numberOfFrames):

            bgTemp = pygame.image.load(str(background) + ".png")
            frameTemp = pygame.image.load("0" + str(state) + str(frame) + ".png")
            bgRect = bgTemp.get_rect()
            frameRect = frameTemp.get_rect()

            screen.fill(black) # erase screen
            screen.blit(bgTemp, bgRect) # draw image within image rectangle
            screen.blit(frameTemp, frameRect) # draw image within image rectangle
            pygame.display.flip() # update changes

            time.sleep(0.5) # delay half a second before moving to next frame
            frame += 1

    prevstate = state

# main function 

# initialize blank screen
screen.fill(black) # erase screen
pygame.display.flip() # update changes

while(1):
    for event in pygame.event.get(): # quit game check
        if event.type == pygame.QUIT: sys.exit()

    # Wait until there is data waiting in the serial buffer
    if(serialPort.in_waiting > 0):

        updateVariables()
        findSound()
        findFrame()
        playAnimation()