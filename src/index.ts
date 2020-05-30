const nativeCopy = require('bindings')('NativeCopy').NativeCopy

export type RecursivePartial<T> = {
    [P in keyof T]?: RecursivePartial<T[P]>;
};

export type CopyRule<T> = { [P in keyof T]: boolean | CopyRule<T[P]> | Array<boolean | CopyRule<T[P]>> };

export function copy<T>(objectOrArrayToCopy: T | Array<T>, rule: CopyRule<T>): RecursivePartial<T> {
    return nativeCopy(objectOrArrayToCopy, rule);
};