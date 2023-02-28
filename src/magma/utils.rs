#[macro_export]
macro_rules! info {
    () => {
        raylib::trace_log(raylib::prelude::TraceLogLevel::LOG_INFO, "(no message)");
    };
    ($($arg:tt)*) => {
        raylib::trace_log(raylib::prelude::TraceLogLevel::LOG_INFO, &format!("{}", format_args!($($arg)*)));
    };
}

#[cfg(test)]
mod tests {
    fn test_printing(){
        info!("Hello world! {} {}",5, 20);
    }
}
