(function(){'use strict';
 var scenes={clear:{temp:'64°',condition:'Clear',feels:'62°F',humidity:'55%'},rain:{temp:'64°',condition:'Rain',feels:'62°F',humidity:'90%'},snow:{temp:'28°',condition:'Snow',feels:'26°F',humidity:'90%'},night:{temp:'64°',condition:'Clear night',feels:'62°F',humidity:'55%'}};
 var preview=document.getElementById('weather-preview');
 document.querySelectorAll('[data-set-scene]').forEach(function(button){button.addEventListener('click',function(){var name=button.getAttribute('data-set-scene'),s=scenes[name];if(!s||!preview)return;preview.setAttribute('data-scene',name);preview.setAttribute('aria-label','Interface illustration with simulated '+s.condition.toLowerCase()+' weather, '+s.temp.replace('°',' degrees Fahrenheit'));document.getElementById('sample-temp').innerHTML=s.temp+'<small>F</small>';document.getElementById('sample-condition').textContent=s.condition;document.getElementById('sample-feels').textContent=s.feels;document.getElementById('sample-humidity').textContent=s.humidity;document.querySelectorAll('[data-set-scene]').forEach(function(b){b.setAttribute('aria-pressed',String(b===button));});});});
 // A project Pages URL supplies its public owner/repository automatically.
 // For custom hosting set window.RETRO_REPOSITORY = 'owner/repository' before this script.
 var repo=window.RETRO_REPOSITORY||'';
 if(!repo&&/^[a-z0-9-]+\.github\.io$/i.test(location.hostname)){var owner=location.hostname.split('.')[0],parts=location.pathname.split('/').filter(Boolean);repo=owner+'/'+(parts.length?parts[0]:owner+'.github.io');}
 if(!/^[a-z0-9_.-]+\/[a-z0-9_.-]+$/i.test(repo))return;
 var base='https://github.com/'+repo,files={installer:'RetroWeather_Setup_v1.2.1.exe',source:'RetroWeather_v1.2.1_Source.zip',network:'RetroWeather_v1.2.1_Network_Source.zip',checksums:'SHA256SUMS.txt'};
 document.querySelectorAll('[data-repo]').forEach(function(a){a.href=base;});document.querySelectorAll('[data-issues]').forEach(function(a){a.href=base+'/issues/new/choose';});document.querySelectorAll('[data-download]').forEach(function(a){var f=files[a.getAttribute('data-download')];if(f)a.href=base+'/releases/download/v1.2.1/'+f;});
})();
