# Tokenised Marquee
## Marquee Display with Tokenised Access

This project illustrates the power of TokenScript and Token Centric thinking applied to a real world example

Imagine a prestigious advertising location with a digital display. You may be thinking of Times Square, Akihabara, Shanghai's Bund, Heathrow or Dubai airport or even just a digital subway billboard located in a busy station in Sydney Australia.

How would you show your advert there? How would you control when it will be shown? How would you ensure the owners of that space get paid so the billboard stays in place and there is incentive to maintain it? What if you wanted to speculate on access to this billboard to encourage and drive enhancements?

In this example we show a very basic system in order for learning how TokenScript can simplify interfaces with real-world applications.

This sample shows tokenised access to a simple digital marquee which can show a scrolling message. We will go through all the steps to set this up: Minting an ERC721 token contract, setting up a TokenScript for this contract, mint some NFT access tokens using TokenScript, then setting up the marquee with Web3E to allow owners of the token to display messages. 

[![SC2 Video](https://user-images.githubusercontent.com/12689544/147811768-53f65be0-1c50-444d-bd62-259eb0329a5c.gif)](https://youtu.be/qcksv5Vzxfc)

In the second part of this application we will build on the sample to create a system where weekly advertising timeslots are controlled by NFTs, and display adverts are posted by burning an ERC20 currency token.

1. Mint an ERC721 test token using [our automated mint facility](https://jamessmartcell.github.io/) which should work with every wallet that supports WalletConnect. It's best if you use AlphaWallet as we will be using TokenScript functionality later. Pick whichever style of asset suits you. Note that this facility also mints the first token for you so will appear in the wallet.
2. Determine the address of the new contract. If you know your wallet address you can easily look this up in etherscan. Otherwise, in AlphaWallet, go to the Activity page, locate the constructor transaction at the top of the page and click it, then click on the 'extra info' button in the top right (three horizontal lines). You can now click on click the copy button for the address of the 'Message Writer Token (MSG)' 
3. Edit the attached TokenScript to point at the created token.
   - Open the messagetoken.xml TokenScript in this repo.
   - Find line 25 in ```messagetoken.xml```, with the comment ```<!-- Your ERC721 Token address goes here -->```. Copy your new contract address over the zero address
4. Upload this TokenScript to AlphaWallet on your mobile. 
   - Easiest way is to drop it into Telegram's 'Saved Messages' folder, then open Telegram on your phone. Click on the XML and open with AlphaWallet. 
   - You can also use File Explorer to drag/drop the file into the Android 'Downloads' folder, then using the Android file manager click on the new file.
5. Locate the new token. Click on it. You should have 1 token showing. If no tokens show, simply wait for sync and try again.
6. Try the TokenScript 'Mint' verb in the token. You can use this button to mint new NFTs and send them to your friends.
7. Open the Platform.io TokenMarquee project in VSCode or Atom
8. Edit the WiFi password details so it can access your 2.4G WiFi.
9. Connect an ESP32 devkit to your PC and flash the TokenMarquee Firmware from Platform.io
10. Run the Firmware and read the Serial output, you will see something like this:

```
Recovering key from EEPROM
Address: 0x6FD4B1103A05699F00000C657E1F6936F3911111
```
   - This is the device lookup address; the TokenScript bridge requires the IoT device to sign a challenge to ensure your TokenScript connects to the correct device.
   - Copy this address and open up the ```messagetoken.xml``` you edited previously. 
   - We need to tell the TokenScript how to contact your IoT device, so find line 151 which contains a comment ```<--- Your IoT device address goes here```
   - Paste the device IoT address over the zero address on this line.

11. Send the updated TokenScript to your phone via Telegram or as before in step 4.
12. Open the token, check the new 'Message' verb, You should see an input box. Type your message here and click 'Message' at the bottom, sign the message and send.
13. Check the Serial output on your ESP32, you should see the message pop up.
14. Connect up the MAX Panel MAX72XX to the ESP32. I used pins IO2 -> CLK, IO3 -> CS, IO4 -> Data. If you use different pin numbers be sure to change the pin values in the firmware:
![20211231_130808](https://user-images.githubusercontent.com/12689544/147801471-f81f4a85-a3d1-44a6-8a69-2e46ff00588e.jpg)
![20211231_130818](https://user-images.githubusercontent.com/12689544/147801472-60174788-9216-4981-997b-6b2702a9ed0c.jpg)

15. Start the ESP32 and try sending some message text to the Scrolling Display!
16. Send an NFT to a colleague and Telegram the TokenScript. They will be able to post messages on the system.
