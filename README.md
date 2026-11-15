# seatManager
基于C++的简易班级座位排布工具.

## 特性
- 采用基于身高和规则的座位排布方法.
- 默认同桌都是相同性别, 可以在规则中更改.
- 规则支持选择行或者列的范围和距离某人的曼哈顿距离范围, 均支持反选.
- 规则有优先级.
- 首先按照身高排座, 然后根据规则列表, 依次给有规则的学生随机挑选符合规则的位置, 并与被选中位置上的学生交换位置.

## 配置文件
配置文件采用`Json`格式.

### 学生信息配置文件
- 示例:
```json
{
  "students": [
    { "id": 1, "height": 178, "gender": 0, "name": "钊敬彪" },
    { "id": 2, "height": 163, "gender": 1, "name": "彤一诺" },
    { "id": 3, "height": 176, "gender": 0, "name": "溥丹" },
    { "id": 4, "height": 163, "gender": 1, "name": "汉政君" },
    { "id": 5, "height": 162, "gender": 0, "name": "板萍" },
    { "id": 6, "height": 179, "gender": 0, "name": "撒敬阳" },
    { "id": 7, "height": 179, "gender": 0, "name": "阳梓萱" },
    { "id": 8, "height": 175, "gender": 0, "name": "琦艳" },
    { "id": 9, "height": 176, "gender": 0, "name": "布军" },
    { "id": 10, "height": 179, "gender": 1, "name": "天丽芬" }
  ]
}
```
- `students` 数组存放所有学生信息.
- `id` 键定义每个学生id, 大于0不能重复.
- `height` `gender` `name` 定义每个学生的身高, 性别, 姓名.

### 规则信息配置文件
- 示例:
```json
{
    "rules": [
        {
            "id": 1,
            "diffGender": false,
            "L":[{
                "isInverse": false,
                "columnRange": [1, 8],
                "rowRange": [1, 4]
            }]
        },
        {
            "id": 2,
            "diffGender": false,
            "L":[{
                "isInverse": false
            }],
			"C":[{
				"isInverse": true,
				"circle": [20, 8]
			}]
        }
    ]
}
```
- `rules` 数组存放所有规则信息.
- `id` 定义了该条规则在那个学生上生效.
- `diffGender` 定义了该学生是否可以和异性同学坐一起, 不填写默认为 `false`.
- `L` 包含行和列信息, 可选.`isInverse` 定义是否反选. `columnRange` `rowRange` 分别定义列和行范围, 不填写默认全部范围.
- `C` 包含圆信息, 可选.`isInverse` 定义是否反选. `circle` 第一个元素定义以哪个 id 为中心, 第二个元素定义半径.