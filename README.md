# esp32_text_to_speech
 Simply put a speaker on esp32 to read the text with natural speech.
 
## Description

テキストをESP32にしゃべらせる試作品で、ESP32にスピーカーを繋ぐだけで音声がでます。
天気予報や交通情報などを取り込むためにツイッターと連携させてみましたが、IFTTTやBlynkのインターフェースを使用して多様な組み合わせが可能です。
音声合成にはHOYA社の"VoiceText Web API"を使用しました。

Twitter -> IFTTT -> Blynk -> ESP32 -> VoiceText Wev API -> ESP32 (DAC) -> Speaker という組み合わせ経路です。

## Features

- BlynkのWebインターフェースに対応し多彩なコンテンツと連携できる
- 音声合成にはHOYA社の"VoiceText Web API"を使用し、流暢な日本語で話すことができる
- IFTTTやmyThingsなどのサービスを組み合わせができる

## Requirement

- ESP32 x 1
- Speaker x 1
- (2KΩ 半固定抵抗、0.1μ コンデンサ)

- Twitter アカウント : https://twitter.com/
- IFTTT アカウント : https://ifttt.com
- Blynk authキー : https://www.blynk.cc/
- VoiceText Wev API api キー : https://cloud.voicetext.jp/webapi

## Usage

1. IFTTTでTwitterとBlynkを連携させる。Twitterのテキストは "<<<" と ">>>" で囲んでエスケープすること。
2. BlynkのIPアドレスは、「ping blynk-cloud.com」で確認すること。

Make a web requestの例）
http://188.166.206.43/xxxd9xxx00xxx3f6xxx6xxfxxx7e2f7/update/v1?value= <<<{{Text}}>>>

## Installation

- Arduino IDE と Arduino core for the ESP32　でコンパイルし、ESP32に転送
- スピーカーをESP32の GNDとGPIO25(DAC) に接続（だたし抵抗とコンデンサを付けることを推奨）

## Author

kghrlabo (http://kghr.blog.fc2.com/)

## License

TBD

## Memo

音声合成のURLを変更すれば、IBM　の(Watson) Text to Speech や Google のCloud Text-to-Speech なども利用可能なはず。
HOYA社の"VoiceText Web API"の性能はとても高くてほとんど違和感がない。男性の声が選べるのも良いし（現時点で）無料というのも素晴らしい。
