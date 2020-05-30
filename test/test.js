const copy = require('bindings')('NativeCopy').NativeCopy;

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
    arrayTest2: [
        true,
        false,
        true,
        {
            copy: true,
        },
        true
    ],
    arrayTest: {
        copyPlz: true
    },
    deep: {
        plzCopy: true
    }
};

const copied = copy([testObject], rules)

console.log(copied);
console.log(JSON.stringify(copied));