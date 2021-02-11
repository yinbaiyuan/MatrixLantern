// pages/main.js

var app = getApp();

var S_Gif = require("../../utils/libgif.js");
var G_C = require("../../utils/util")
// import S_GIF from '../../utils/libgif.js'

Page({

  /**
   * 页面的初始数据
   */
  data: {
    btName:'加载GIF图',
    submit:'解析',
    sliderW:12,
    sliderH:12,
    btsubmit_disable:true
  },

  /**
   * 生命周期函数--监听页面加载
   */
  onLoad: function (options) {

  },

  /**
   * 生命周期函数--监听页面初次渲染完成
   */
  onReady: function () {

  },

  /**
   * 生命周期函数--监听页面显示
   */
  onShow: function () {
    app.globalData.colorArr = [];
    app.globalData.frameArr = [];
  },

  /**
   * 生命周期函数--监听页面隐藏
   */
  onHide: function () {

  },

  /**
   * 生命周期函数--监听页面卸载
   */
  onUnload: function () {

  },

  /**
   * 页面相关事件处理函数--监听用户下拉动作
   */
  onPullDownRefresh: function () {

  },

  /**
   * 页面上拉触底事件的处理函数
   */
  onReachBottom: function () {

  },

  /**
   * 用户点击右上角分享
   */
  onShareAppMessage: function () {

  },

  sliderchange:function(options)
  {
    let vId = options.currentTarget.id;
    if(vId == 'sliderW')
    {
      this.setData({
        sliderW:options.detail.value
      })
    }else if(vId == 'sliderH')
    {
      this.setData({
        sliderH:options.detail.value
      })
    }
  },

  gifParse : function(path)
  {
    let that = this
    console.log({path})
    wx.getFileSystemManager().readFile({
      filePath:path,
      success(res)
      {
        let data = res.data
        console.log(data)
        let buf = ''
        let step = 10240
        for (let index = 0; index < data.byteLength; index += step) {
          let end = index + step
          if(end > data.byteLength)
          {end = data.byteLength;}
          buf += String.fromCharCode.apply(null, new Uint8Array(data.slice(index, end)));
        }
        var sGfi = new S_Gif();
        var pWidth = that.data.sliderW
        var pHeight = that.data.sliderH
        var colorArr = []
        var frameArr = []
        // data[5]
        sGfi.load_raw(buf,(gifs,frames)=>{
          for (let i = 0; i < frames.frames.length; i++) {
            // for (let i = 0; i < 1; i++) {
            let frame = frames.frames[i]
            // console.log(frame)
            let xOffset = parseInt(frame.w / pWidth)
            let yOffset = parseInt(frame.h / pHeight)
            // console.log({xOffset:xOffset,yOffset:yOffset})
            let frameMatrix = []
            let index = 0
            for(let y = 0 ; y < pHeight ; y++)
            {
              for(let x = 0; x < pWidth ; x++)
              {
                let xpix = parseInt(xOffset / 2) + xOffset * x 
                let ypix = parseInt(yOffset / 2) + yOffset * y
                let ipix = ypix * frame.w + xpix
                // console.log({x:xpix,y:ypix,i:ipix})
                let colorR = frame.data.data[ipix * 4]
                let colorG = frame.data.data[ipix * 4 + 1]
                let colorB = frame.data.data[ipix * 4 + 2]
                if(colorR + colorG + colorB > 0)
                {
                  let color = [colorR,colorG,colorB]
                  // console.log({color})
                  let colorIndex = G_C.colorArrayIndexOf(colorArr,color)
                  if(colorIndex == -1)
                  {
                    colorArr.push(color)
                    colorIndex = colorArr.length - 1
                  }
                  frameMatrix[index++] = y * 16 + x
                  frameMatrix[index++] = colorIndex
                }
              }
            }
            frameArr[i] = frameMatrix
          }
          
          app.globalData.colorArr = colorArr;
          app.globalData.frameArr = frameArr;
          wx.hideLoading()
          wx.showModal({
            title: '提示',
            content: '解析成功，是否返回上一页',
            success (res) {
              if (res.confirm) {
                wx.navigateBack({
                  delta: 1,
                })
              }
            }
          })
          
        })
      }   
    }) 
  },

  Search: function () {

    var that = this
    wx.chooseImage({
      count: 1,
      sizeType: ['original', 'compressed'],
      sourceType: ['album', 'camera'],
      success (res) {
        // tempFilePath可以作为img标签的src属性显示图片
        const tempFilePaths = res.tempFilePaths
        let fileArr = tempFilePaths[0].split('.')
        // console.log(fileArr)
        if(fileArr[fileArr.length-1].toLowerCase() != 'gif')
        {
          wx.showModal({
            title: '错误',
            content: '请选择GIF格式图片',
            showCancel: false
          })
        }else{
          that.setData({
            imgPath:tempFilePaths[0]
          })
          that.setData({
            btsubmit_disable:false
          })
        }

        
      }
    })
  },
  submit: function() {
    var that = this
    wx.showLoading({
      title: '正在解析',
    })
    setTimeout(function () {
      wx.hideLoading()
    }, 10000)
    that.gifParse(that.data.imgPath)
  }
})