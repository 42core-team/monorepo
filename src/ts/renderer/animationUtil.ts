type timingCurvePoints = { realTime: number; animationProgress: number }[];

abstract class TimingCurve {
	protected abstract curve: timingCurvePoints;
	public getValue(t: number): number {
		if (t < 0 || t > 1) throw new RangeError(`t (${t}) must be between 0 and 1 inclusive`);
		const pts = this.curve;
		if (pts.length === 0) return t;
		if (t <= pts[0].realTime) return pts[0].animationProgress;
		const last = pts[pts.length - 1];
		if (t >= last.realTime) return last.animationProgress;
		for (let i = 0; i < pts.length - 1; i++) {
			const a = pts[i],
				b = pts[i + 1];
			if (t >= a.realTime && t <= b.realTime) {
				const u = b.realTime === a.realTime ? 1 : (t - a.realTime) / (b.realTime - a.realTime);
				return a.animationProgress + u * (b.animationProgress - a.animationProgress);
			}
		}
		return last.animationProgress;
	}
}

class LinearTimingCurve extends TimingCurve {
	protected curve: timingCurvePoints = [
		{ realTime: 0, animationProgress: 0 },
		{ realTime: 1, animationProgress: 1 },
	];
}

class EaseInOutTimingCurve extends TimingCurve {
	protected curve: timingCurvePoints = [
		{ realTime: 0, animationProgress: 0 },
		{ realTime: 0.25, animationProgress: 0.125 },
		{ realTime: 0.5, animationProgress: 0.5 },
		{ realTime: 0.75, animationProgress: 0.875 },
		{ realTime: 1, animationProgress: 1 },
	];
}

class MidTickIncreaseTimingCurve extends TimingCurve {
	protected curve: timingCurvePoints = [
		{ realTime: 0.4, animationProgress: 0 },
		{ realTime: 0.6, animationProgress: 1 },
	];
}
