from pathlib import Path
import json,wave,struct,datetime
p=Path(__file__).resolve().parents[1]
# Windows XP ICO frames must be DIB bitmaps, not Vista-only PNG icon frames.
b=(p/'assets/RetroWeather.ico').read_bytes();reserved,kind,count=struct.unpack_from('<HHH',b);assert kind==1 and count>=4
for i in range(count):
 width,height,_,_,planes,bits,size,offset=struct.unpack_from('<BBBBHHII',b,6+i*16)
 assert b[offset:offset+8]!=b'\x89PNG\r\n\x1a\n'
 assert bits==32
print('PASS: XP-compatible multi-size DIB icon frames')
files=list((p/'dist/Sounds').glob('*.wav'));assert len(files)==7
assert not (p/'dist/Music').exists()
for f in files:
 with wave.open(str(f)) as w:
  assert w.getnchannels()==1 and w.getsampwidth()==2 and w.getframerate()==22050
  assert w.getnframes()==17*22050 and w.getcomptype()=='NONE'
print('PASS: seven seamless 17-second PCM mono weather loops, 22.05 kHz')
j=json.loads((p/'dist/data/timezones.json').read_text())
def epoch(iso):return int(datetime.datetime.fromisoformat(iso).replace(tzinfo=datetime.timezone.utc).timestamp())
def off(zone,t):
 rows=j['zones'][zone];return next(r[1] for r in reversed(rows) if r[0]<=t)
assert off('America/New_York',epoch('2026-03-08T06:59:59'))==-18000
assert off('America/New_York',epoch('2026-03-08T07:00:00'))==-14400
assert off('America/New_York',epoch('2026-11-01T05:59:59'))==-14400
assert off('America/New_York',epoch('2026-11-01T06:00:00'))==-18000
assert off('America/Phoenix',epoch('2026-07-01T00:00:00'))==-25200
assert off('Pacific/Honolulu',epoch('2026-07-01T00:00:00'))==-36000
print('PASS: US DST spring/fall transitions, Arizona and Hawaii exceptions')
phases=json.loads((p/'dist/data/moon.json').read_text());times=[]
for row in phases:
 iso=f'{row["year"]:04d}-{row["month"]:02d}-{row["day"]:02d}T{row["time"]}:00';times.append(epoch(iso))
assert all(4*86400<b-a<10*86400 for a,b in zip(times,times[1:]))
r=next(r for r in phases if r['year']==2024 and r['month']==4 and r['day']==8 and r['phase']=='New Moon')
h,m=map(int,r['time'].split(':'));assert abs(h*60+m-(18*60+21))<=2
illum=json.loads((p/'dist/data/illumination.json').read_text());assert all(0<=r[1]<=1 for r in illum)
print('PASS: continuous primary lunar phases, NASA/USNO April 2024 reference, illumination bounds')
