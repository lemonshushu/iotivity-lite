# IoTivity 실험 방법

라즈베리파이를 서버로, 컴퓨터를 클라이언트로 사용하는 환경을 가정한다.

본 실험에서는 IoTivity를 이용하여 작성한 fasten_server, fasten_client 앱을 이용하여, onboarding_tool을 이용하여 두 앱의 provisioning을 진행한다.

fasten_server, fasten_client의 코드는 `fasten` 디렉토리에 포함되어 있다. (`fasten/fasten_server.c`, `fasten/fasten_client.c`)

## 1. Build IoTivity

IoTivity와, fasten_server, fasten_client, onboarding_tool을 빌드하는 과정에 대한 설명이다.

아래 과정들을 라즈베리파이(서버), 컴퓨터(클라이언트)에 모두 동일하게 진행한다.

먼저, repository를 클론한다.

```bash
$ git clone https://github.com/lemonshushu/iotivity-lite.git
```

그런 다음, port/linux 디렉토리로 이동한 뒤, `make fasten`을 입력하여 fasten_server, fasten_client, onboarding_tool을 빌드한다. 빌드가 완료되면, `port/linux` 디렉토리에 `fasten_server`, `fasten_client` 실행파일들이 생성된다.

```bash
$ cd iotivity-lite/port/linux
$ make fasten
```

만약 troubleshooting을 위해 디버그 메시지를 보고 싶다면, 아래와 같이 `-e DEBUG=1`을 포함하여 빌드한다. (단, DEBUG 옵션으로 빌드할 경우에는, 일반 빌드와 비교하여 실험 측정시 latency가 2배정도로 증가하므로, 실제 실험 시에는 반드시 DEBUG 옵션을 끄고 빌드하도록 한다.)

```bash
$ make -e DEBUG=1 fasten
```

## 2. Onboarding & Provisioning

이 과정을 진행하기에 앞서, **반드시 라즈베리파이와 컴퓨터가 같은 로컬 네트워크 안에 있도록 하고**, 각각의 **와이파이를 켠다.** 와이파이 외에, 이더넷 등 다른 IP 네트워크 인터페이스는 꺼져 있도록 유의한다.

또한, 방화벽에 의해 mDNS가 차단되어 있으면 discovery가 이루어지지 않으므로, **일시적으로 방화벽을 끄거나, 방화벽 설정에서 mDNS를 허용하도록 한다.**

### 2-1. Run fasten_server, fasten_client, onboarding_tool

**라즈베리파이에서** fasten_server를 실행한다.

```bash
# On Raspberry Pi
$ ./fasten_server
```

그런 다음, **컴퓨터에서** fasten_client를 실행한다. fasten_client는 command line argument로 payload size를 받는데, 여기서는 onboarding & provisioning 용도로만 쓸 것이므로 아무 수나 입력한다. (아래 예시에서는 0)

```bash
# On Computer
$ ./fasten_client 0
[DEBUG] Issued GET request at 1699515602.346713
```

다음으로, **컴퓨터에서** 새로운 터미널을 열고, onboarding_tool을 실행한다.  그러면 아래와 같은 메뉴가 뜬다.

```bash
$ ./onboarding_tool
Started device with ID: 2f3335bf-90a1-4beb-5618-b1ab9712fdb6


################################################
OCF 2.x Onboarding Tool
################################################
[0] Display this menu
-----------------------------------------------
[1] Discover un-owned devices
[2] Discover un-owned devices in the realm-local IPv6 scope
[3] Discover un-owned devices in the site-local IPv6 scope
[4] Discover owned devices
[5] Discover owned devices in the realm-local IPv6 scope
[6] Discover owned devices in the site-local IPv6 scope
[7] Discover all resources on the device
-----------------------------------------------
[8] Just-Works Ownership Transfer Method
[9] Request Random PIN from device for OTM
[10] Random PIN Ownership Transfer Method
[11] Manufacturer Certificate based Ownership Transfer Method
-----------------------------------------------
[12] Provision pairwise credentials
[13] Provision ACE2
[14] Provision auth-crypt RW access to NCRs
[15] RETRIEVE /oic/sec/cred
[16] DELETE cred by credid
[17] RETRIEVE /oic/sec/acl2
[18] DELETE ace by aceid
[19] RETRIEVE own creds
[20] DELETE own cred by credid
[21] Provision role RW access to NCRs
[22] Provision identity certificate
[23] Provision role certificate
[24] Provision pairwise OSCORE contexts
[25] Provision Client Group OSCORE context
[26] Provision Server Group OSCORE context
[27] Set security domain info
-----------------------------------------------
[96] Install new manufacturer trust anchor
[97] RESET device
[98] RESET OBT
-----------------------------------------------
[99] Exit
################################################

Select option:
```

### 2-2. Device Discovery

이제부터는 onboarding tool에서 옵션을 입력하여 server와 client의 ownership 및 credential 설정을 해줄 것이다.

먼저, device를 discover하기 위해서 `1`을 입력하고 엔터를 친다. 정상적으로 discover되었다면 몇 초 안에 아래와 같이 디바이스가 두 개 뜰 것이다(각각 server, client):

```
Discovered unowned device: a948167f-91fd-499c-6970-48fcc9785826 at:
coap://[fe80:0000:0000:0000:c8b3:89d4:6222:c268%2]:41003
coaps://[fe80:0000:0000:0000:c8b3:89d4:6222:c268%2]:34115
coap+tcp://[fe80:0000:0000:0000:c8b3:89d4:6222:c268%2]:47647
coaps+tcp://[fe80:0000:0000:0000:c8b3:89d4:6222:c268%2]:53955

Discovered unowned device: 664c97fa-28c9-4a15-6703-10664accb3c4 at:
coap://[fe80:0000:0000:0000:0d5c:09c7:2523:b253%2]:57348
coaps://[fe80:0000:0000:0000:0d5c:09c7:2523:b253%2]:32812
coap+tcp://[fe80:0000:0000:0000:0d5c:09c7:2523:b253%2]:51205
coaps+tcp://[fe80:0000:0000:0000:0d5c:09c7:2523:b253%2]:45151
```

### 2-3. Ownership Transfer

`[8] Just-Works Ownership Transfer Method`을 이용하여 두 디바이스의 소유권을 획득할 것이다.

`8`을 입력한다. 그러면 아래와 같이 두 디바이스가 뜬다.

```
Unowned Devices:
[0]: a948167f-91fd-499c-6970-48fcc9785826 - FASTEN Client
[1]: 664c97fa-28c9-4a15-6703-10664accb3c4 - FASTEN Server

Select device:
```

Client의 소유권을 먼저 획득해보자. `0`을 입력한다. 

```
Select device: 0

Successfully issued request to perform ownership transfer
...
Successfully performed OTM on device with UUID 11410468-8cd7-4251-5126-45ffbe6790ad
```

OTM(Ownership Transfer Method)가 성공적으로 수행되었다면 위와 같은 메시지가 뜬다.

동일한 방법으로 Server의 소유권도 획득한다:

```
8

Unowned Devices:
[0]: 664c97fa-28c9-4a15-6703-10664accb3c4 - FASTEN Server


Select device: 0

Successfully issued request to perform ownership transfer
...
Successfully performed OTM on device with UUID ffb3d193-7274-4b94-6b09-b15327cfbc63

```

### 2-4. Provision ACL on Server

서버 resource의 ACL(Access Control List)를 설정해줄 것이다.

`14`를 입력하고 엔터를 친다.

```
14

Provision auth-crypt * ACE
My Devices:
[0]: 11410468-8cd7-4251-5126-45ffbe6790ad - FASTEN Client
[1]: ffb3d193-7274-4b94-6b09-b15327cfbc63 - FASTEN Server


Select device for provisioning:
```

두 디바이스 중에서, Server에 해당하는 디바이스의 번호를 입력한다. (여기서는 `1`)

```
Select device for provisioning: 1

Successfully issued request to provision auth-crypt * ACE
...
Successfully provisioned auth-crypt * ACE to device ffb3d193-7274-4b94-6b09-b15327cfbc63
```

성공적으로 ACL이 설정되었다.

### 2-5. Issue Certificate Credentials for Server, Client

서버와 클라이언트에게 certificate credential을 발급해줄 것이다. (참고: OCF specification은 mutual authentication을 전제로 하므로 client에게도 certificate을 발급해주어야 한다.)

 `22`를 입력한다.

```
22

My Devices:
[0]: 11410468-8cd7-4251-5126-45ffbe6790ad - FASTEN Client
[1]: ffb3d193-7274-4b94-6b09-b15327cfbc63 - FASTEN Server

Select device: 
```

Client의 certificate부터 발급해보자. `0`을 입력한다.

```
Select device: 0

Successfully issued request to provision identity certificate
...
Successfully provisioned identity certificate
```

성공적으로 발급되었다. 마찬가지 방법으로 Server에게도 certificate을 발급해준다.

```
22

My Devices:
[0]: 11410468-8cd7-4251-5126-45ffbe6790ad - FASTEN Client
[1]: ffb3d193-7274-4b94-6b09-b15327cfbc63 - FASTEN Server

Select device: 1

Successfully issued request to provision identity certificate
...
Successfully provisioned identity certificate
```

모든 과정이 완료되었다. onboarding_tool, fasten_server, fasten_client를 종료한다.

Server/client의 credential은 각각 `port/linux/fasten_server_creds`, `port/linux/fasten_client_creds` 디렉토리에 [CBOR](https://en.wikipedia.org/wiki/CBOR) 형식으로 저장된다. 따라서 server와 client를 종료하고 재실행해도 credential은 유지된다.

## 3. Usage of fasten_server & fasten_client

#### fasten_server

-   사용법 : `./fasten_server` (별도의 command line argument를 받지 않음)

-   동작
    -   한번 실행되면, 종료될 때까지 계속해서 루프를 돌며 client로부터 request를 받고 그에 해당하는 데이터를 보내준다.
    -   Client는 `payload-size`를 쿼리 파라미터로 보내게 되는데, 서버는 이에 해당하는 용량만큼의 데이터를 응답으로 돌려준다.

#### fasten_client

-   사용법 : `./fasten_client <payload-size>`
    -   `payload-size` : 서버로부터 전송받을 데이터의 크기. 단위는 byte. 최소 `0`, 최대 `10485760`(10MiB)

-   동작
    -   command line argument로 받은 `payload-size`를 쿼리 파라미터로 포함하여 서버에게 요청을 보낸다. 서버는 그에 해당하는 사이즈의 데이터를 응답으로 보내주게 된다.
    -   실행되면, 딱 한 번만 서버에게 request를 보내고, 그에 대해 응답을 받은 뒤에는 아무런 동작을 하지 않는다. 자동으로 종료되지는 않으므로 `Ctrl+c`를 눌러 직접 종료시켜 줘야 한다.

## 4. Experiment

이제 본격적으로 fasten_server와 fasten_client를 이용해서 실험할 것이다.

라즈베리파이에서 fasten_server를 실행한다. (`2. Onboarding & Provisioning` 단계 완료 후, 반드시 fasten_server와 fasten_client를 종료 후 재실행해야 올바른 실험이 가능하다.)

```bash
# On Raspberry Pi
$ ./fasten_server
```

fasten_server의 경우 한 번만 실행하여 계속 켜 둘 것이고, fasten_client의 경우 한 번 실행 당 요청 한 번이므로, 새로운 요청을 보내고자 할 때마다 종료 후 재실행해야 한다. (이렇게 한 이유는, 앱을 종료시키지 않고 계속해서 서버와 통신하게 하면, TLS handshake가 새로 이루어지지 않고, 기존의 session을 resume하기 때문이다.)

(서버가 켜져 있는 상태에서) 원하는 payload size를 설정하여 client를 실행한다. 그러면 다음과 같이 **client-side에** 타임스탬프가 찍힌다.

```bash
$ ./fasten_client 1024
[DEBUG] Issued GET request at 1699518476.027299
[DEBUG] TLS handshake started at 1699518476.035961
[DEBUG] TLS handshake ended at 1699518476.035987
[DEBUG] Packet received at 1699518476.368077
[DEBUG] Packet received at 1699518476.410836
```

**모든 타임스탬프의 단위는 초(s)이다.** 각 타임스탬프에 관한 설명은 다음과 같다:

-   `[DEBUG] Issued GET request at xxx.xxx`
    -   Client가 GET request를 issue하는 시점이다.
    -   위치 : `fasten/fasten_client.c` (`oc_do_get()` 호출 직전) — [코드 보기](https://github.com/lemonshushu/iotivity-lite/blob/153f0417375ac06d3eb983786077cc5a73973db4/fasten/fasten_client.c#L74-L76)
-   `TLS handshake started at xxx.xxx`
    -   TLS handshake가 시작되는 시점이다.
    -   위치 : `security/oc_tls.c` (`mbedtls_ssl_handshake()` 호출 직전) — [코드 보기](https://github.com/lemonshushu/iotivity-lite/blob/2d2dcd4b22df52bf2c658c84824df639062f74f8/security/oc_tls.c#L2600-L2602)
-   `TLS handshake ended at xxx.xxx`
    -   TLS handshake가 끝나는 시점이다.
    -   위치 : `security/oc_tls.c` (`mbedtls_ssl_handshake()` 리턴 직후) — [코드 보기](https://github.com/lemonshushu/iotivity-lite/blob/2d2dcd4b22df52bf2c658c84824df639062f74f8/security/oc_tls.c#L2604-L2606)
-   `Packet received at xxx.xxx`
    -   TCP 소켓에서 데이터를 읽어올 때마다 찍힌다.
    -   Payload size에 상관없이 두 번씩 찍히는데, **두 번째 timestamp가 실제 클라이언트가 요청한 데이터에 해당한다.** (첫 번째는 무시)
    -   위치 : `messaging/coap/coap.c` (coap_tcp_parse_message()`의 진입점) — [코드 보기](https://github.com/lemonshushu/iotivity-lite/blob/e21fd4552ab6b69bfa00f7d9160618b95a70416f/messaging/coap/coap.c#L1511-L1513)

`Ctrl+c`를 눌러 클라이언트를 종료한다.

## Resetting Devices

Server 또는 Client의 credential을 삭제하고 재설정하고 싶다면 `port/linux/fasten_server_creds` 또는 `port/linux/fasten_client_creds` 디렉토리 내부의 모든 파일을 삭제하면 된다.

Onboarding tool의 `[97] RESET device` 옵션도 같은 동작을 수행한다.

`make clean`만 하면 object 파일들만 삭제되고, credential 파일들은 그대로 유지된다. `make cleanall`을 하면 모든 앱들의 모든 credential들이 삭제된다.

Credential이 삭제되면 [2. Onboarding & Provisioning](#2. Onboarding & Provisioning)의 과정을 다시 진행해야 한다.
