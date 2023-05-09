# Формат XML

Для передачи сетевой модели от клиента будет использоваться XML файл. Структура файла должна описывать:
  - Название модели
  - Настройки модели
    - Популирование таблиц маршрутизации
    - Длительность симуляции
    - Точность времени

  - Элементы сети - `<node>`
    - их названия
    - Сетевые устройства - `<device>`rm 
      - тип сетевого устройства `type`
      - название - `name`
      - адреса
        - IPv4/6
      - параметры - `<attributes>`
    
    - Приложения
      - тип приложения - `type`
      - название - `name`
      - параметры приложения - `<attributes>`
    
    - параметры интернет-стека
  
  - Соединения
    - соединение - `<connection>`
      - список соединяемых сетевых устройств (минимум два для Csma) - `<interfaces>`
      - название соединения - `name`
      - тип соединения - `type`
      - параметры соединения - `<attributes>`

  - Регистраторы трафика
    - Путь до TraceSource вида
      - `/node_name/device_name/$ns3::DropTailQueue/Tx`
      - `/csma_connection_1/$ns3::CsmaChannel/Rx`
      - и т.д.
    - Тип регистратора статистики
      - Регистратор пакетов
      - Регистратор ещё чего-то
      - Тут скорее нужно лучше изучить модуль сбора статистики у ns3
    - названия файла назначения
 
 ------------------------------------------------------------------------------

## `<model>`
Корневой элемент модели

Параметры модели:
<!-- TODO: описать параметры - популирование таблиц, установка длительности и тд -->

```xml
<model name="CsmaNetworkModel">
  <node-list> ... </node-list>
  <statistics> ... </statistics>
  ...
</model>
```

Как вариант, можно использовать в виде атрибута.

## `<node-list>`
Тег описания списка элементов сети, содержит в себе подэлементы типа `<node>`.

```xml
<node-list>
  <node> ... </node>
  <node> ... </node>
</node-list>
```

## `<node>`
Тег описания элемента сети, содержащий следующие атрибуты:
  - `name` - описание имени элемента сети
  - `<device-list>` - список сетевых устройств
  - `<applications>` - список приложений
  - `<routing>` - таблица маршрутизации
  - `<internet-stack>` - настройки стека интернет (возможно такого не будет)

```xml
<node name="Client">
  <device-list> ... </device-list>
  <applications> ... </applications>
  <routing> ... </routing>
</node>
```

## `<device-list>`
Список сетевых устройств 

```xml
<device-list>
  <device> ... </device>
  <device> ... </device>
  ...
</device-list>
```

## `<device>`
Описание сетевого интерфейса 

Атрибуты:
  - `id` - порядковый номер интерфейса на устройстве
  - `name` - имя
  - `type` - тип (Csma, PointToPoint, ...)

Вложенные поля:
`<address>` - адрес интерфейса
`<attributes>` - список атрибутов ns3 и их значений

```xml
<device name="eth0" 
        type="Csma">  
  <address value="10.01.22.222" netmask="255.255.255.0"/>
  <address value="2022:dead:beef:2023::0" prefix="64"/>

  <attribures>
    <attribute key="Address" value="AB:CD:EF:01:02:03">
    <attribute key="Mtu" value="1200"/>
    <attribute key="EncapsulationMode" value="Llc"/>
  </attribures>
</device>
```

## `<routing>`
Список маршрутов для элемента сети

```xml
<routing>
  <route network="10.101.0.0" prefix="16" dst="eth0" metric="10"/>
  <route network="10.101.0.0" prefix="16" dst="eth1" metric="20"/>
  <route network="2001:dead:beef:1002::0" prefix="64" dst="eth1" metric="30"/>
</routing>
```

### `<route>`
Атрибуты:
  - `network` - адрес назначений как ipv4, так и ipv6
  - `prefix` - префикс сети ipv4/ipv6
  - `dst` - интерфейс назначения
  - `metric` - величина метрики пути

```xml
<route network="10.101.0.0" prefix="16" dst="eth1" metric="20"/>
<route network="2001:dead:beef:1002::0" prefix="64" dst="eth1" metric="30"/>
```

### `<application>`
Описание приложений-генератор трафика
Атрибуты:
  - `name` - имя
  - `type` - тип

Вложенные параметры:
  - `<attributes>`

```xml
<node>
  <application name="echo_server" type="UdpEchoServer">
    <attributes>
      <attribute key="RemoteAddress" value="1.1.1.1"/>
      <attribute key="RemotePort" value="2222"/>
    </attributes>
  </application>
</node>
```


## `<connections>`
Список подключений между интерфейсами сети

## `<connection>`
Канал передачи между сетевыми интерфейсами

Атрибуты:
  - `id` - порядковый номер
  - `name` - имя
  - `type` - тип канала

Вложенные параметры:
  - `<interfaces>` - список соединенных сетевых интерфейсов
  - `<attributes>` - список ns-3 атрибутов соединения

```xml
<connections>
  <connection name="nodea/eth0-to-nodeb/eth1" 
              type="Csma">
    <interfaces>
      <interface>node1/eth0</interface>
      <interface>node1/eth1</interface>
    </interfaces>

    <attribures>
      <attribute key="DataRate" value="2Mbps"/>
      <attribute key="Delay" value="1ms"/>
    </attribures>
  </connection>
</connections>
```

## `<statistics>`
Описание списка регистраторов статистики
Регистраторы статистики представляют из себя что-то похожее на Probe из ns-3 - цепляются
к определенному TraceSource и транслирует специфичные для вида формат в CSV файл (тут возможно 
можно воспользоваться возможностью кеширования данных для уменьшения частоты записи)

Атрибуты:
  - `source` - источник данных, представленный в виде пути до TraceSource
  - `type` - тип регистратора статистики
  - `file` - название файла назначения
  - `start` (опциональный) - время инициализации и начала работы регистратора
  - `end` (опциональный) - время завершения работы регистратора


```xml
<statistics>
  <registrator source="/Names/node-a/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow"
               type="ns3::Uinteger32Probe"
               sink="Output"
               file="node-a-cwnd"
               value_name="cwnd"
               start="1s"
               end="10s"/>
</statistics>
```

## Пример
```xml
<?xml version="1.0" encoding="UTF-8"?>
<model name="UdpEchoExperiment">
  <populate-routing-tables>true</populate-routing-tables>
  <duration>10s</duration>
  <precision>NS</precision>

  <node-list>
    <node name="Client">
      <device-list>
        <device name="eth0" 
                type="Csma" >  
          <address value="10.01.22.222" netmask="255.255.255.0"/>

          <attribures>
            <attribute key="Address" value="AB:CD:EF:01:02:03"/>
            <attribute key="Mtu" value="1200"/>
            <attribute key="EncapsulationMode" value="Llc"/>
          </attribures>
        </device>
      </device-list>

      <applications>
        <application name="EchoClient" type="ns3::UdpEchoClientApplication">
          <attributes>
            <attribute key="Port" value="666">
            <attribute key="Remote" value="10.01.22.2">
          </attributes>
        </application>
      </applications>
    </node>

    <node name="Server">
      <device-list>
        <device name="eth0" type="Csma">
          <address value="10.01.22.2" netmask="255.255.255.0"/>
            <attribures>
              <attribute key="Mtu" value="1200"/>
              <attribute key="EncapsulationMode" value="Llc"/>
            </attribures>
        </device>
      </device-list>
      
      <applications>
        <application name="EchoServer" type="ns3::UdpEchoServerApplication">
          <attributes>
            <attribute key="Port" value="666">
            <attribute key="Remote" value="10.01.22.222">
          </attributes>
        </application>
      </applications>
    </node>
  </node-list>

  <connections>
    <connection name="Client-to-Server" type=Csma>
      <interfaces>
        <interface>Client/eth0</interface>
        <interface>Server/eth0</interface>
      </interfaces>
    </connection>
  </connections>

  <statistics>
    
  </statistics>
</model>
```