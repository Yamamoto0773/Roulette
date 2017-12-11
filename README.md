# Roulette
寮祭の抽選会で使うルーレットアプリです

### 概要
このルーレットアプリでは、1から最大999までの番号をMT法を用いてランダムに表示します。
また、番号の範囲によって男子用のくじと女子用のくじを区別できるようになっています。
くじ番号の範囲はテキストファイルで指定します。

描画にはDirectX9.0を用いています。[こちらのサイト](http://www.charatsoft.com/develop/otogema/)内に掲載されている音楽ゲームのソースコードを改変して作成しました。

### 主な機能
* 表示するくじ番号の範囲(最小値、最大値)を指定できる機能
* ルーレット中の数字を任意の桁から1桁ずつとめる機能
	* もちろん全ての桁を1度に停止できます
* 次に表示するくじ番号を限定できる機能(男子用くじ、女子用くじのどちらかだけを表示できる)


### ライセンス (about License)
(This software is released under the BSD 2-clause "Simplified" License, see LICENSE.md)

このアプリはBSD 2-clause "Simplified" Licenseのもとで公開されています。
BSD 2-clause "Simplified" LicenseについてはLICENSE.mdを参照して下さい。
ざっくり説明すると以下のようになっています。

<dl>
	<dt>必須</dt>
	<dd>著作権の表示</dd>
	<dt>許可</dt>
	<dd>商用利用</dd>
	<dd>修正</dd>
	<dd>配布</dd>
	<dd>個人利用</dd>
	<dt>禁止</dt>
	<dd>責任免除</dd>
</dl>

  
  
-------------

#### コミットメッセージの書式
1行目:コミットの種類

2行目:改行

3行目:コミットの要約


#### コミットの種類
add     新規追加

fix     バグ修正

update  バグではない変更

disable コメントアウトなど、機能の無効化

remove  ファイルやコードの削除

clean   ファイルやコードの整理

