Pilotage d'une maison connectée<br>
-------------------------------------<br><br>
Equipements pilotés:
- un premier système d'irrigation composé d'un réservoir de 200L rempli par un puit
- une commande de lance d'arrosage
- un deuxième système d'irrigation commandé par une électrovanne pilotée
- une VMC avec marche lente/rapide
- des équipements de cuisine (plaques, four, micro-ondes)
- une PAC<br>
L'ensemble communique également avec Home Assistant via des messages MQTT

Système de commande:<br>
- Carte electonique ES32A08<br>
https://fr.aliexpress.com/item/1005006185382987.html?spm=a2g0o.productlist.main.1.698elv0Ylv0YwW&algo_pvid=f51ea843-ec14-4632-9974-88f5128b477e&algo_exp_id=f51ea843-ec14-4632-9974-88f5128b477e-0&pdp_npi=4%40dis%21EUR%2140.96%2133.69%21%21%21313.64%21257.97%21%4021039fae17272463435504450e2d37%2112000036180720579%21sea%21FR%210%21ABX&curPageLogUid=gOixLiNgnswU&utparam-url=scene%3Asearch%7Cquery_from%3A<br>
  Nota:<br>
  Le reglage de la PAC est effectué par un système déporté (voir plus loin)<br>
  La marche rapide de la VMC est effectué par un système déporté (voir plus loin)<br>
<br>  
- Pilotage<br>
  La carte ES32A08 est accédée par une application Android via des messages MQTT par l'intermediaire
  d'un courtier Mosquito tournant dans Home Assistant.<br>
  Nota : le courtier MQTT peut également être fourni par un esp32 ou un Raspberry PI.<br>
  Il est possible de programmer la carte ES32A08 avec tout systeme permettant d'envoyer des messages MQTT.<br><br><br>
  
Application Android<br>
https://github.com/geo17137/HomeControlFrontEnd
<br><br>
Plans<br>
------<br>
- Distribution d'eau<br>
https://github.com/geo17137/HomeControl/blob/master/DocsPdf/Plan%20distribution%20eau.pdf

- Câblage armoire de commande<br>
https://github.com/geo17137/HomeControl/blob/master/DocsPdf/Plan%20de%20cablage%20armoire%20automate%20ESP32%20-%20V6.pdf

- Câblage carte de commande<br>
https://github.com/geo17137/HomeControl/blob/master/DocsPdf/CablageES32A08.pdf

- Documentation générale<br>
https://github.com/geo17137/HomeControl/blob/master/DocsPdf/DocGeneraleHomeControl.pdf

Codes et equipements annexes déportés<br>
--------------------------------------------<br>
- Application Android<br>
https://github.com/geo17137/HomeControlAndroidFrontEnd

Périphériques déportés<br>
-------------------------<br>
- Commande PAC<br>
https://github.com/geo17137/MitsubishiPAC_IR<br>
- Carte de commande IR PAC<br>
https://fr.aliexpress.com/item/1005005777409596.html?spm=a2g0o.order_list.order_list_main.247.15e75e5bc2Vkbf&gatewayAdapt=glo2fra

- Commande VMC rapide<br>
https://github.com/geo17137/VmcSpeedUp
- Carte relais Wifi<br>
https://fr.aliexpress.com/item/1005003493100800.html?spm=a2g0o.productlist.main.3.4d062676Fq4FnN&algo_pvid=a803610d-f58a-49a4-b435-8789b4cb9b29&algo_exp_id=a803610d-f58a-49a4-b435-8789b4cb9b29-1&pdp_npi=4%40dis%21EUR%212.46%212.46%21%21%212.68%212.68%21%40213db7b317269055681243227e4b9a%2112000026040971359%21sea%21FR%21917560485%21X&curPageLogUid=KEOAsm3pATLN&utparam-url=scene%3Asearch%7Cquery_from%3A
- Alimentation 220/5V de la carte<br>
https://fr.aliexpress.com/item/32408565688.html?spm=a2g0o.order_list.order_list_main.321.24865e5bui7fWN&gatewayAdapt=glo2fra

