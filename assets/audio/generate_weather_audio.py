from pathlib import Path
import numpy as np
from scipy.signal import butter,sosfilt
from scipy.io.wavfile import write
import subprocess,json
OUT=Path(__file__).resolve().parents[2]/"dist"/"Sounds"
OUT.mkdir(parents=True,exist_ok=True)
SR=22050;SECONDS=18;N=SR*SECONDS
rng=np.random.default_rng(728183)
def noise(lo,hi,n=N):
 x=rng.normal(0,1,n)
 if lo and hi:sos=butter(3,[lo,hi],btype='bandpass',fs=SR,output='sos')
 elif hi:sos=butter(3,hi,btype='lowpass',fs=SR,output='sos')
 else:sos=butter(3,lo,btype='highpass',fs=SR,output='sos')
 x=sosfilt(sos,x);return x/max(np.std(x),1e-8)
def gust(n=N,rate=.1):
 t=np.arange(n)/SR;return .57+.18*np.sin(2*np.pi*rate*t+.6)+.12*np.sin(2*np.pi*rate*.43*t+1.7)
def wind(strength=1,wintry=False):
 t=np.arange(N)/SR
 x=noise(45,420)*gust()*strength*.08+noise(350,1800)*gust(rate=.19)*strength*.025
 if wintry:x+=noise(1500,3600)*gust(rate=.14)*.015
 return x
def event(x,start,y):
 at=int(start*SR);n=min(len(y),len(x)-at)
 if n>0:x[at:at+n]+=y[:n]
def rain(heavy=False):
 t=np.arange(N)/SR
 x=(noise(550,7000)*.055+noise(110,1500)*.045)*(1+.07*np.sin(t*.45))
 # A diffuse shower over soft patter, without hard roof impacts.
 for i in range(580 if heavy else 350):
  length=rng.uniform(.014,.065);n=int(length*SR);u=np.arange(n)/SR
  drop=noise(700,6500,n)*np.exp(-u/(length*.25))*np.minimum(u/.002,1)
  event(x,rng.uniform(0,SECONDS-.1),drop*rng.uniform(.006,.026))
 return x*(1.18 if heavy else 1)
def birds(x):
 for start in [1.9,5.1,9.6,14.0]:
  for k in range(2 if start!=9.6 else 3):
   dur=.19+rng.uniform(0,.08);u=np.arange(int(SR*dur))/SR
   f=2200+650*np.sin(np.pi*u/dur)+300*u/dur
   phase=2*np.pi*np.cumsum(f)/SR
   y=(np.sin(phase)+.12*np.sin(2*phase))*(np.sin(np.pi*u/dur)**1.7)*.033
   event(x,start+k*.32,y)
 return x
def crickets(x):
 for series,carrier in enumerate([3350,4200]):
  for start in np.arange(.6+series*.8,SECONDS-1,2.35+series*.55):
   for k in range(5):
    dur=.045;u=np.arange(int(SR*dur))/SR
    y=np.sin(2*np.pi*carrier*u+3*np.sin(2*np.pi*90*u))*np.sin(np.pi*u/dur)**2
    event(x,float(start+k*.11),y*(.012 if series else .021))
 return x
def thunder(x,start,duration=6,amp=.15):
 n=int(duration*SR);u=np.arange(n)/SR
 rumble=noise(27,240,n)*.75+noise(180,850,n)*.25
 env=(1-np.exp(-u*2.5))*np.exp(-u/.95)+.4*(1-np.exp(-u))*np.exp(-u/2.1)
 env*=.8+.2*np.sin(u*10)+.1*np.sin(u*24)
 tail=np.minimum(1,(duration-u)/.8)
 event(x,start,rumble*env*tail*amp)
 return x
tracks=[('Clear Day',birds(wind(.55))),('Clear Night',crickets(wind(.45))),('Cloudy',wind(1.05)),('Rain',rain()),('Snow',wind(.75,True)),('Thunderstorm',thunder(thunder(rain(True),3.3,6,.19),11.2,5,.13)),('Fog',wind(.38))]
# One common gain preserves differences in intensity across weather conditions.
peak=max(float(np.max(np.abs(x))) for _,x in tracks);gain=min(1,.83/peak)

manifest=[]
for i,(name,x) in enumerate(tracks):
 x=x*gain
 m=SR
 a=np.linspace(0,1,m,endpoint=False)
 # Crossfade the end into the beginning without preview fade-to-silence gaps.
 x=np.concatenate((x[m:-m],x[-m:]*(1-a)+x[:m]*a))
 out=OUT/(str(i+1).zfill(2)+'_'+name.replace(' ','_')+'.wav')
 write(out,SR,np.round(x*32767).astype(np.int16))
 manifest.append({'file':out.name,'seconds':len(x)/SR,'sample_rate':SR,'rms':float(np.sqrt(np.mean(x*x))),'peak':float(np.max(np.abs(x)))})
(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2))
print('Generated seven seamless 17-second weather loops.')
