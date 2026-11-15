from pathlib import Path
import subprocess,tempfile,shutil,sys
root=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='retro-weather-tests-') as tmp:
 temp=Path(tmp);audio=temp/'audio';audio.mkdir()
 for f in (root/'dist/Sounds').glob('*.wav'):shutil.copyfile(f,Path(str(audio)+'\\'+f.name))
 count=0
 for f in sorted((root/'tests').glob('test_*.cpp')):
  stub='audio_stubs' if f.stem=='test_weather_audio' else 'render_stubs' if f.stem=='test_backbuffer' else 'stubs'
  exe=temp/(f.stem+('.exe' if sys.platform=='win32' else ''))
  subprocess.run(['g++','-std=c++11','-O2','-I'+str(root/'tests'/stub),str(f),'-o',str(exe)],check=True)
  subprocess.run([str(exe)]+([str(audio)] if f.stem=='test_weather_audio' else []),check=True)
  count+=1
 subprocess.run([sys.executable,str(root/'tests/check_assets.py')],check=True)
 print('PASS:',count,'C++ regression suites and asset checks')
