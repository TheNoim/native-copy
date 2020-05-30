# native-copy

This native nodejs addon copies objects according to a rule specified by you. 

### Example

```javascript
const { copy } = require('native-copy');

const testObject = {
    copy: 1,
    doNotCopy: 0,
    arrayTest: [
        {
            copyPlz: 55555
        }
    ],
    arrayTest2: [
        1,
        2,
        3,
        {
            copy: 4,
            notCopy: 3
        },
        8
    ],
    deep: {
        plzCopy: 5,
        dont: 33
    }
};

const rules = {
    copy: true,
    arrayTest: {
        copyPlz: true
    },
    arrayTest2: [
        true,
        false,
        true,
        {
            copy: true,
        },
        true
    ],
    deep: {
        plzCopy: true
    }
};

const copied = copy([testObject], rules)
```

The `copied` variable will look like this:

```json
[
	{
		"copy": 1,
		"arrayTest2": [
			1,
			3,
			{
				"copy": 4
			},
			8
		],
		"arrayTest": [
			{
				"copyPlz": 55555
			}
		],
		"deep": {
			"plzCopy": 5
		}
	}
]
```