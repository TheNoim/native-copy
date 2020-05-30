const nativeBindings = require('bindings')('NativeCopy');
const nativeCopy = nativeBindings.NativeCopy;
const nativeConvert = nativeBindings.ConvertRule;

export type RecursivePartial<T> = {
    [P in keyof T]?: RecursivePartial<T[P]>;
}

export type CopyRule<T> = { [P in keyof T]: boolean | CopyRule<T[P]> | Array<boolean | CopyRule<T[P]>> };

export function copy<T>(objectOrArrayToCopy: T | Array<T>, rule: CopyRule<T> | string[]): RecursivePartial<T> {
    if (Array.isArray(rule)) {
        return nativeCopy(objectOrArrayToCopy, convertPathArrayToCopyRule(rule));
    } else {
        return nativeCopy(objectOrArrayToCopy, rule);
    }
}

export function convertPathArrayToCopyRule<T = unknown>(pathArray: string[]): CopyRule<T> {
    return nativeConvert(pathArray);
}
