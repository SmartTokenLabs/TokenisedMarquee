# Tokenised Marquee
## Marquee Display with Tokenised Access

This project illustrates the power of TokenScript and Token Centric thinking applied to a real world example

Imagine a prestigious advertising location with a digital display. You may be thinking of Times Square, Akihabara, Shanghai, Dubai airport or even just a digital subway billboard located in a busy station in Sydney Australia.

How would you show your advert there? How would you control when it will be shown? How would you ensure the owners of that space get paid so the billboard stays in place and there is incentive to maintain it? What if you wanted to speculate on access to this billboard to encourage and drive enhancements?

In this example we show a very basic system in order for learning how TokenScript can simplify interfaces with real-world applications.

This sample shows tokenised access to a simple digital marquee which can show a scrolling message. We will go through all the steps to set this up: Minting an ERC721 token contract, setting up a TokenScript for this contract, mint some NFT access tokens using TokenScript, then setting up the marquee with Web3E to allow owners of the token to display messages. 

In the second part of this application we will build on the sample to create a system where weekly advertising timeslots are controlled by NFTs, and display adverts are posted by burning an ERC20 currency token.

1. Mint an ERC721 token that has the ```mintUsingSequentialTokenId()``` function for simplified minting (Our test TokenScript uses this function).
2. Edit the attached TokenScript to point at the created token.
3. Mint a few ERC721 tokens on this new contract easily using TokenScript.
4. Deploy the Firmware on the ESP32 hardware and determine the hardware lookup address.
