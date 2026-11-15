from pathlib import Path
from datetime import datetime,timedelta,timezone
from zoneinfo import ZoneInfo
import json
p=Path(__file__).resolve().parents[1]/'dist'/'data'
zones=[]
for line in Path('/usr/share/zoneinfo/zone.tab').read_text().splitlines():
 if not line.startswith('#') and line.split('\t')[0] in ['US','PR','VI','GU','MP','AS','FM','MH','PW']:zones.append(line.split('\t')[2])
zones+=['UTC','America/Atikokan','America/Creston','Pacific/Chuuk','Pacific/Pohnpei','America/Kralendijk','America/Lower_Princes']
start=int(datetime(2020,1,1,tzinfo=timezone.utc).timestamp());end=int(datetime(2051,1,1,tzinfo=timezone.utc).timestamp())
out={}
for zone in sorted(set(zones)):
 tz=ZoneInfo(zone)
 def off(t):return int(datetime.fromtimestamp(t,timezone.utc).astimezone(tz).utcoffset().total_seconds())
 rows=[[start,off(start)]];a=start
 while a<end:
  b=min(a+86400,end);o=off(b)
  if o!=rows[-1][1]:
   lo=a;hi=b
   while hi-lo>1:
    mid=(lo+hi)//2
    if off(mid)==rows[-1][1]:lo=mid
    else:hi=mid
   rows.append([hi,o])
  a=b
 out[zone]=rows
(p/'timezones.json').write_text(json.dumps({'start':start,'end':end,'zones':out},separators=(',',':')))
print('Timezone transitions ready:',len(out),'zones, 2020-2050',flush=True)
