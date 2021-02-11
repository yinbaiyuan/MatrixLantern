const formatTime = date => {
  const year = date.getFullYear()
  const month = date.getMonth() + 1
  const day = date.getDate()
  const hour = date.getHours()
  const minute = date.getMinutes()
  const second = date.getSeconds()

  return [year, month, day].map(formatNumber).join('/') + ' ' + [hour, minute, second].map(formatNumber).join(':')
}

const formatNumber = n => {
  n = n.toString()
  return n[1] ? n : '0' + n
}

const colorArrayIndexOf = (arr, color) =>
{
  for(let i = 0 ; i<arr.length ; i++ )
  {
    if(arr[i][0] == color[0] && arr[i][1] == color[1] && arr[i][2] == color[2])
    {
      return i;
    }
  }
  return -1;
}

module.exports = {
  formatTime: formatTime,
  colorArrayIndexOf:colorArrayIndexOf
}
