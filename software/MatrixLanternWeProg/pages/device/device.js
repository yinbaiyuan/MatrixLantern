const app = getApp()
Page({
  data: {
    inputText: 'mootek',
    receiveText: '',
    name: '',
    connectedDeviceId: '',
    services: {},
    characteristics: {},
    connected: true,
    funcList:[{funcId:1,type:1,name:"彩虹",desc:"彩虹效果",enable:true,icon:'rainbow.png'},
              {funcId:2,type:1,name:"火焰",desc:"火焰效果",enable:true,icon:'fire.png'},
              {funcId:3,type:1,name:"随机点亮",desc:"一个点一个点将矩阵填满",enable:true,icon:'random.png'},
              {funcId:4,type:1,name:"条纹",desc:"条纹效果",enable:true,icon:'line.png'},
              {funcId:5,type:1,name:"文字",desc:"文字效果",enable:true,icon:'text.png'},
              {funcId:6,type:1,name:"噪点",desc:"噪点效果",enable:true,icon:'noise.png'},
              {funcId:7,type:2,name:"像素动画",desc:"将GIF图片导入为像素动画",enable:false,icon:'pixels.png'}
            ]
  },
  bindInput: function (e) {
    this.setData({
      inputText: e.detail.value
    })
    console.log(e.detail.value)
  },
  Send: function () {
    var that = this
    let funcList = that.data.funcList
    if (that.data.connected) {
      var transData = []
      var transDataLength = 0;//总长度4字节
      transData[transDataLength++] = funcList[0].enable?1:0
      transData[transDataLength++] = funcList[1].enable?1:0
      transData[transDataLength++] = funcList[2].enable?1:0
      transData[transDataLength++] = funcList[3].enable?1:0
      transData[transDataLength++] = funcList[4].enable?1:0
      transData[transDataLength++] = funcList[5].enable?1:0
      transData[transDataLength++] = funcList[6].enable?1:0
      
      if(funcList[6].enable)
      {
        transData[transDataLength++] = app.globalData.colorArr.length 
        for(let cCount = 0;cCount<app.globalData.colorArr.length;cCount++)
        {
          let color = app.globalData.colorArr[cCount]
          transData[transDataLength++] = color[0]
          transData[transDataLength++] = color[1]
          transData[transDataLength++] = color[2]
        }
        transData[transDataLength++] = app.globalData.frameArr.length
        for(let fCount = 0 ;fCount<app.globalData.frameArr.length;fCount++)
        {
          let frame = app.globalData.frameArr[fCount]
          transData[transDataLength++] = frame.length >> 8 & 0xFF
          transData[transDataLength++] = frame.length & 0xFF
          for(let dCount = 0;dCount<frame.length;dCount++)
          {
            transData[transDataLength++] = frame[dCount]
          }
        }
      }
      // console.log(transData)
      var buffer = new ArrayBuffer(transData.length)
      var dataView = new Uint8Array(buffer)
      for (var i = 0; i < transData.length; i++) {
        dataView[i] = transData[i]
      }
      wx.showLoading({
        title: '正在发送',
      })
      setTimeout(function () {
        wx.hideLoading()
      }, 10000)
      wx.writeBLECharacteristicValue({
        deviceId: that.data.connectedDeviceId,
        serviceId: that.data.services[0].uuid,
        characteristicId: that.data.characteristics[1].uuid,
        value: buffer,
        success: function (res) {
          console.log('发送指令成功:'+ res.errMsg)
          wx.hideLoading()
          wx.showModal({
            title: '数据发送成功',
            content: ''
          })        
        },
        fail: function (res) {
          // fail
          //console.log(that.data.services)
          wx.hideLoading()
          console.log('message发送失败:' +  res.errMsg)
          wx.showToast({
            title: '数据发送失败，请稍后重试',
            icon: 'none'
          })
        }       
      })
    }
    else {
      wx.showModal({
        title: '提示',
        content: '蓝牙已断开',
        showCancel: false,
        success: function (res) {
          that.setData({
            searching: false
          })
        }
      })
    }
  },
  onLoad: function (options) {
    var that = this
    console.log(options)
    that.setData({
      name: options.name,
      connectedDeviceId: options.connectedDeviceId
    })
    wx.getBLEDeviceServices({
      deviceId: that.data.connectedDeviceId,
      success: function (res) {
        console.log(res.services)
        that.setData({
          services: res.services
        })
        wx.getBLEDeviceCharacteristics({
          deviceId: options.connectedDeviceId,
          serviceId: res.services[0].uuid,
          success: function (res) {
            console.log(res.characteristics)
            that.setData({
              characteristics: res.characteristics
            })
            wx.notifyBLECharacteristicValueChange({
              state: true,
              deviceId: options.connectedDeviceId,
              serviceId: that.data.services[0].uuid,
              characteristicId: that.data.characteristics[0].uuid,
              success: function (res) {
                console.log('启用notify成功：' + that.data.characteristics[0].uuid)
                console.log(JSON.stringify(res));
                that.onBLECharacteristicValueChange();
              },
              fail: function () {
                console.log('开启notify失败' + that.characteristicId)
              }
            })
          }
        })
      }
    })
    wx.onBLEConnectionStateChange(function (res) {
      console.log(res.connected)
      that.setData({
        connected: res.connected
      })
    })

  },
  onReady: function () {

  },
  onShow: function () {
    let list = this.data.funcList;
    if(app.globalData.colorArr.length>0)
    {
      list[list.length-1].enable = true
    }else{
      list[list.length-1].enable = false
    }
    this.setData(
    {
      funcList:list
    }
    )
  },
  onHide: function () {

  },
  onBLECharacteristicValueChange: function() {
    var that = this;
      wx.onBLECharacteristicValueChange(function(res) {
      var receiveText = app.buf2string(res.value)
      console.log('监听低功耗蓝牙设备的特征值变化事件成功');
      console.log(app.buf2string(res.value));
      that.setData({
        receiveText: receiveText
      })
    })
  },
  switchChange:function(options)
  {
    let funcId = options.currentTarget.id;
    this.data.funcList[funcId-1].enable = options.detail.value
    console.log(this.data.funcList)
  },

  gifParse:function(options) {
    if(options.detail.value)
    {
      wx.navigateTo({
        url: '../gifParse/gifParse'
      })
    }else
    {
      app.globalData.colorArr = [];
      app.globalData.frameArr = [];
    }

  }
})

