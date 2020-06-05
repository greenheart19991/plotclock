// --- helper functions ---

const toDegrees = (rAngle) => {
    return rAngle * (180 / Math.PI);
};

const toRadians = (dAngle) => {
    return dAngle * (Math.PI / 180);
};

// get 3rd side of triangle using 2 sides and angle between them
// note: angle in degrees
const getSideBy2SidesAndAngle = (a, b, dAngle) => {
    if (dAngle < 0 || dAngle > 180) {
        throw new RangeError(
            `Incorrect angle: ${dAngle}.\n` +
            'Angle of triangle must be in range [0, 180]'
        );
    }

    const rAngle = toRadians(dAngle);
    return Math.sqrt(a * a + b * b - 2 * a * b * Math.cos(rAngle));
};

// get angle between 'a' and 'b' sides
// note: angle in degrees
const getAngleBy3Sides = (a, b, c) => {
    if (
        (a + b) < c
        || (a + c) < b
        || (b + c) < a
    ) {
        throw new Error(`Triangle a=${a}, b=${b}, c=${c} doesn't exist`);
    }

    const rAngle = Math.acos((a * a + b * b - c * c) / (2 * a * b));
    return toDegrees(rAngle);
};

// --- offsets ---
// feel free to customize

const LK = 0; // any value
const OL = 0; // >= 0

// --- design settings ---
// feel free to customize
// note: angles in degrees,
// distances in coordinate plane units

const AB = 25.6;
const AF = 35;
const BG = 35;
const FC = 45;
const CG = 45;
const CD = 13.2;
const DCF = 133.5;

// --- boundary values ---
// feel free to customize

const maxFCG = 150;
const maxCFA = 170;
const maxCGB = 170;

// --- calculated constants ---

const FD = getSideBy2SidesAndAngle(CD, FC, DCF);
const LA = LK + AF;

// --- algorithm function ---

// get marker position relative to the origin.
// Result is in format of { x, y }.
const getMarkerPosition = (servo1DAngle, servo2DAngle) => {
    if (
        servo1DAngle < 0 || servo1DAngle > 180
        || servo2DAngle < 0 || servo2DAngle > 180
    ) {
        return null;
    }

    const ABG = 180 - servo2DAngle;
    const AG = getSideBy2SidesAndAngle(AB, BG, ABG);
    const GAB = getAngleBy3Sides(AG, AB, BG);
    const FAG = servo1DAngle - GAB;

    if (FAG < 0) {
        return null;
    }

    const FG = getSideBy2SidesAndAngle(AF, AG, FAG);
    if (FG > (FC + CG) || getAngleBy3Sides(FC, CG, FG) > maxFCG) {
        return null;
    }

    const AFG = getAngleBy3Sides(FG, AF, AG);
    const FB = getSideBy2SidesAndAngle(AF, AB, servo1DAngle);

    if (
        getAngleBy3Sides(FB, AB, AF) > ABG
        || GAB > servo1DAngle
    ) {
        return null;
    }

    const FGB = getAngleBy3Sides(FG, BG, FB);

    const CFG = getAngleBy3Sides(FC, FG, CG);
    const CGF = CFG;
    const CFA = CFG + AFG;
    const CGB = CGF + FGB;

    if (CFA > maxCFA || CGB > maxCGB) {
        return null;
    }

    const CA = getSideBy2SidesAndAngle(FC, AF, CFA);
    const FCA = getAngleBy3Sides(FC, CA, AF);
    const FAC = 180 - (CFA + FCA);

    const leftDCA = DCF + FCA;

    const DCA = leftDCA <= 180
        ? leftDCA
        : 360 - leftDCA;

    const DA = getSideBy2SidesAndAngle(CD, CA, DCA);
    const DAC = getAngleBy3Sides(DA, CA, CD);

    const FAD = leftDCA <= 180
        ? FAC - DAC
        : FAC + DAC;

    const LAF = 180 - servo1DAngle;
    const LAD = LAF + FAD;

    const DN = DA * Math.sin(toRadians(LAD));
    const NA = DA * Math.cos(toRadians(LAD));

    const x = LA - NA;
    const y = DN + OL;

    return { x, y };
};

// get angles of servo1 and servo2 for specified marker position.
// Result is in format of { servo1, servo2 }, in degrees.
const getServoAngles = (x, y) => {
    const DN = y - OL;

    if (DN < 0) {
        return null;
    }

    const NA = Math.abs(LA - x);
    const AD = Math.sqrt(DN * DN + NA * NA);
    const NAD = toDegrees(Math.atan(DN / NA));

    const DFA = getAngleBy3Sides(FD, AF, AD);
    const DFC = getAngleBy3Sides(FD, FC, CD);
    const CFA = DFA - DFC;

    if (CFA > maxCFA) {
        return null;
    }

    const CA = getSideBy2SidesAndAngle(FC, AF, CFA);
    const FAC = getAngleBy3Sides(AF, CA, FC);
    const FCA = 180 - CFA - FAC;

    const DAC = getAngleBy3Sides(AD, CA, CD);
    const FAD = (DCF + FCA <= 180)
        ? FAC - DAC
        : FAC + DAC;

    const FAB = x < LA
        ? FAD + (180 - NAD)
        : FAD + NAD;

    if (FAB > 180) {
        return null;
    }

    const CAB = FAB - FAC;
    const CB = getSideBy2SidesAndAngle(CA, AB, CAB);

    const ABC = getAngleBy3Sides(AB, CB, CA);

    const CBG = getAngleBy3Sides(CB, BG, CG);

    const CGB = getAngleBy3Sides(CG, BG, CB);
    if (CGB > maxCGB) {
        return null;
    }

    const ABG = ABC + CBG;
    if (ABG > 180) {
        return null;
    }

    const ACB = 180 - CAB - ABC;
    const BCG = 180 - CBG - CGB;
    const FCG = FCA + ACB + BCG;

    if (FCG > maxFCG) {
        return null;
    }

    return {
        servo1: FAB,
        servo2: 180 - ABG
    };
};

document.addEventListener('DOMContentLoaded', () => {

    // --- test program ---

    let points = [];

    for (let servo1DAngle = 180; servo1DAngle >= 0; servo1DAngle--) {
        for (let servo2DAngle = 180; servo2DAngle >= 0; servo2DAngle--) {
            const position = getMarkerPosition(servo1DAngle, servo2DAngle);
            if (position !== null) {
                points.push(position);
            }
        }
    }

    // --- show points on page ---

    // If measure unit isn't specified by name
    // (un or px prefix doesn't exist), it is in pixels.
    // Only point's X and Y values are measured in coordinate
    // plane units (and point's minX, maxX etc respectively)

    // pixels in 1 coordinate plane unit
    // (display purposes only)
    const pxInUn = 5;

    // coordinate plane paddings when drawing on canvas,
    // in coordinate plane units
    const cpPaddingHorizontalUn = 5;
    const cpPaddingVerticalUn = 5;

    const cpPaddingHorizontalPx = cpPaddingHorizontalUn * pxInUn;
    const cpPaddingVerticalPx = cpPaddingVerticalUn * pxInUn;

    const xs = points.map((v) => v.x);
    const ys = points.map((v) => v.y);

    const minX = Math.min(...xs);
    const maxX = Math.max(...xs);
    const maxY = Math.max(...ys);

    const canvasWidth = (maxX + (minX < 0 ? -minX : 0)) * pxInUn + 2 * cpPaddingHorizontalPx;
    const canvasHeight = maxY * pxInUn + 2 * cpPaddingVerticalPx;

    const fromTopToXAxis = maxY * pxInUn + cpPaddingVerticalPx;
    const fromLeftToYAxis = minX < 0
        ? -(minX * pxInUn - cpPaddingHorizontalPx)
        : cpPaddingHorizontalPx;

    const chart = document.getElementById('chart');
    const cx = chart.getContext('2d');

    cx.imageSmoothingEnabled = false;

    cx.canvas.width = canvasWidth;
    cx.canvas.height = canvasHeight;

    // draw grid

    cx.beginPath();
    cx.lineWidth = 1;
    cx.strokeStyle = 'lightgray';
    
    const firstXLineOffset = fromLeftToYAxis % pxInUn;
    const firstYLineOffset = fromTopToXAxis % pxInUn;

    // '- 0.5' is a fix to draw a tiny 1px line
    for (let x = firstXLineOffset - 0.5; x <= canvasWidth; x += pxInUn) {
        cx.moveTo(x, 0);
        cx.lineTo(x, canvasHeight);
    }

    for (let y = firstYLineOffset - 0.5; y <= canvasHeight; y += pxInUn) {
        cx.moveTo(0, y);
        cx.lineTo(canvasWidth, y);
    }

    cx.stroke();

    // draw axes

    cx.beginPath();
    cx.lineWidth = 1;
    cx.strokeStyle = 'black';

    // draw y axis
    cx.moveTo(fromLeftToYAxis - 0.5, cpPaddingVerticalPx);
    cx.lineTo(fromLeftToYAxis - 0.5, fromTopToXAxis);

    // draw x axis
    cx.moveTo(cpPaddingHorizontalPx, fromTopToXAxis - 0.5);
    cx.lineTo(canvasWidth - cpPaddingHorizontalPx, fromTopToXAxis - 0.5);

    // draw arrows of y axis
    cx.moveTo(fromLeftToYAxis - 0.5, cpPaddingVerticalPx);
    cx.lineTo(fromLeftToYAxis - 4 - 0.5, cpPaddingVerticalPx + 4);
    cx.moveTo(fromLeftToYAxis - 0.5, cpPaddingVerticalPx);
    cx.lineTo(fromLeftToYAxis + 4 + 0.5, cpPaddingVerticalPx + 4);

    // draw arrows of x axis
    cx.moveTo(canvasWidth - cpPaddingHorizontalPx, fromTopToXAxis - 0.5);
    cx.lineTo(canvasWidth - cpPaddingHorizontalPx - 4, fromTopToXAxis - 4 - 0.5);
    cx.moveTo(canvasWidth - cpPaddingHorizontalPx, fromTopToXAxis - 0.5);
    cx.lineTo(canvasWidth - cpPaddingHorizontalPx - 4, fromTopToXAxis + 4 + 0.5);

    cx.stroke();

    // draw axes markup (10un marks)

    cx.beginPath();
    cx.lineWidth = 1;
    cx.strokeStyle = 'black';

    const pxInUnDec = 10 * pxInUn;
    const firstMarkX = cpPaddingHorizontalPx + ((fromLeftToYAxis - cpPaddingHorizontalPx) % pxInUnDec);

    // draw X axis markup
    // -4 - arrows length
    // -2 in loop condition - margin from latest mark to arrow
    for (let x = firstMarkX; x <= canvasWidth - cpPaddingHorizontalPx - 4 - 2; x += pxInUnDec) {
        if (x === fromLeftToYAxis) {
            continue;
        }

        // marks length = 2
        cx.moveTo(x - 0.5, fromTopToXAxis - 2);
        cx.lineTo(x - 0.5, fromTopToXAxis + 2);
    }

    // draw Y axis markup
    for (let y = fromTopToXAxis - pxInUnDec; y >= cpPaddingVerticalPx; y -= pxInUnDec) {
        cx.moveTo(fromLeftToYAxis - 2, y - 0.5);
        cx.lineTo(fromLeftToYAxis + 2, y - 0.5);
    }

    cx.stroke();

    // draw servo shafts

    cx.beginPath();
    cx.fillStyle = 'black';

    const cpOriginX = fromLeftToYAxis;
    const cpOriginY = fromTopToXAxis;

    const servo1ShaftX = cpOriginX + (LK + AF) * pxInUn;
    const servo1ShaftY = cpOriginY - OL * pxInUn;
    const servo2ShaftX = servo1ShaftX + AB * pxInUn;
    const servo2ShaftY = servo1ShaftY;

    cx.arc(servo1ShaftX, servo1ShaftY, 4, 0, Math.PI * 2);
    cx.arc(servo2ShaftX, servo2ShaftY, 4, 0, Math.PI * 2);
    cx.fill();

    // draw points

    const drawPoint = (cpXUn, cpYUn) => {
        const cpXPx = cpXUn * pxInUn;
        const cpYPx = cpYUn * pxInUn;

        const x = cpOriginX + cpXPx;
        const y = cpOriginY - cpYPx;

        cx.beginPath();
        cx.fillStyle = 'red';

        cx.arc(x, y, 2, 0, Math.PI * 2);
        cx.fill();
    };

    points.forEach((v) => drawPoint(v.x, v.y));
});
