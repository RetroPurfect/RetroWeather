"""Build-time only: pip install PyMeeus==0.5.12. No Python needed by the player."""
from pymeeus.Moon import Moon
from pymeeus.Epoch import Epoch
from datetime import datetime,timedelta,timezone
from pathlib import Path
import json
p=Path(__file__).resolve().parents[1]/'dist'/'data'
start=datetime(2019,12,1,tzinfo=timezone.utc);end=datetime(2051,2,1,tzinfo=timezone.utc)
phases={};at=start
while at<end:
 for target,name in [('new','New Moon'),('first','First Quarter'),('full','Full Moon'),('last','Last Quarter')]:
  phase=Moon.moon_phase(Epoch(at.year,at.month,at.day),target)
  yy,mm,dd=phase.get_date();utc=(phase.jde()-2440587.5)*86400-Epoch.tt2ut(yy,mm)
  dt=datetime.fromtimestamp(round(utc/60)*60,timezone.utc)
  phases[(int(round(utc/60)),name)]={'phase':name,'year':dt.year,'month':dt.month,'day':dt.day,'time':dt.strftime('%H:%M')}
 at+=timedelta(days=7)
rows=[v for k,v in sorted(phases.items())];(p/'moon.json').write_text(json.dumps(rows,separators=(',',':')))
print('Offline moon phases:',len(rows),'2020-2050 coverage',flush=True)
illum=[];at=datetime(2020,1,1,tzinfo=timezone.utc)
while at<end:
 ep=Epoch(at.year,at.month,at.day);ep+=Epoch.tt2ut(at.year,at.month)/86400
 illum.append([int(at.timestamp()),round(Moon.illuminated_fraction_disk(ep),5)])
 at+=timedelta(days=1)
(p/'illumination.json').write_text(json.dumps(illum,separators=(',',':')))
print('Daily illumination samples:',len(illum))
