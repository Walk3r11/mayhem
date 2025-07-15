import type { AppProps } from 'next/app';
import '../styles/Home.module.css';
import '../styles/globals.css';
import { useEffect } from 'react';

function MyApp({ Component, pageProps }: AppProps) {
    useEffect(() => {
        const cursor = document.getElementById('custom-cursor');
        if (!cursor) return;
        const moveCursor = (e: MouseEvent) => {
            cursor.style.transform = `translate3d(${e.clientX - 20}px, ${e.clientY - 20}px, 0)`;
        };
        document.body.style.cursor = 'none';
        window.addEventListener('mousemove', moveCursor);
        return () => {
            window.removeEventListener('mousemove', moveCursor);
            document.body.style.cursor = '';
        };
    }, []);
    return <Component {...pageProps} />;
}

export default MyApp; 